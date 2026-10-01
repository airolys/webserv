<?php
if (isset($_COOKIE['username'])) {
    $username = htmlspecialchars($_COOKIE['username']);
    echo "<h1>Welcome back, $username!</h1>";
} else {
    if ($_SERVER['REQUEST_METHOD'] == 'POST' && isset($_POST['username'])) {
        $username = htmlspecialchars($_POST['username']);
        setcookie('username', $username, time() + (60), "/"); // 30 days
        echo "<h1>Welcome, $username!</h1>";
    } else {
        echo '<form method="POST" action="">
                <label for="username">Enter your name:</label>
                <input type="text" id="username" name="username" required>
                <input type="submit" value="Submit">
              </form>';
    }
}
?>
