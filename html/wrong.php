#!/usr/bin/php
<?php
// This script runs indefinitely in a loop, useful for testing timeout functionality

set_time_limit(0); // Disable PHP's max execution time limit

while (true) {
    // Just output some data every second to simulate activity
    echo "Still running... " . date('Y-m-d H:i:s') . "\n";
    flush(); // Send output to the client immediately
    sleep(1); // Sleep for 1 second
}
?>