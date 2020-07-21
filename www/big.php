<?php

echo "SALUT\n";

for ($i = 0; $i < 60000000; ++$i) {
	echo "$i\n";
}

file_put_contents("im_done", "thanks");