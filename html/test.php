<?php
header("Content-Type: text/html");
echo "<html><body>";
echo "<h1>PHP CGI Test</h1>";
echo "<p>Environment Variables:</p>";
foreach($_SERVER as $key => $value) {
    echo "<p>$key: $value</p>";
}
echo "</body></html>";
?>