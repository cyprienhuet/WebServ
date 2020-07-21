#!/usr/bin/perl

local ($buffer, @pairs, $pair, $name, $value, %FORM);
# Read in text

print "Status: 200\r\n";
print "Content-type:text/plain\r\n\r\n";

$ENV{'REQUEST_METHOD'} =~ tr/a-z/A-Z/;

if ($ENV{'REQUEST_METHOD'} eq "POST") {
   while (read(STDIN, $buffer, 100)) {
		;
   }
} else {
   $buffer = $ENV{'QUERY_STRING'};
}

for (my $i=0; $i <= 70000000; $i++) {
   print "$i\n";
}

print "done";

1;
