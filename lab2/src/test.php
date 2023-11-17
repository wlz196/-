<!DOCTYPE html>
<html>
<head>
    <title>test</title>
</head>
<body>
    <h2>用户注册</h2>
    <form action="test.php" method="post">
        用户名: <input type="text" name="username"><br>
        密码: <input type="password" name="password"><br>
        <input type="submit" name="register" value="注册">
        <input type="submit" name="login" value="登录">
    </form>

    <?php
    if ($_SERVER["REQUEST_METHOD"] == "POST") {
        $conn = new mysqli("localhost", "debian-sys-maint", "plmOKTXGSephK7Bq", "users");

        if ($conn->connect_error) {
            die("连接失败：" . $conn->connect_error);
        }

        $username = $_POST['username'];
        $password = $_POST['password'];

        if (isset($_POST['register'])) {
            $check_query = "SELECT * FROM user_info WHERE username='$username'";
            $result = $conn->query($check_query);

            if ($result->num_rows > 0) {
                echo "用户名已被注册";
            } else {
                $insert_query = "INSERT INTO user_info (username, password) VALUES ('$username', '$password')";
                if ($conn->query($insert_query) === TRUE) {
                    echo "注册成功";
                } else {
                    echo "Error: " . $insert_query . "<br>" . $conn->error;
                }
            }
        } elseif (isset($_POST['login'])) {
            $check_query = "SELECT * FROM user_info WHERE username='$username' AND password='$password'";
            $result = $conn->query($check_query);

            if ($result->num_rows > 0) {
                header("Location: login_success.php");
            } else {
                echo "无此用户，请注册";
            }
        }

        $conn->close();
    }
    ?>
</body>
</html>
