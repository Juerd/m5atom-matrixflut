use Net::MQTT::Simple "test.mosquitto.org";

my $x =
    "%...!" .
    "%...." .
    "%%%.!" .
    "%.%.!" .
    "%.%.!";

$x =~ s/%/\0\x80\0/g;  # green
$x =~ s/!/\x80\0\0/g;  # red
$x =~ s/\./\0\0\0/g;

publish "matrixflut" => $x;
