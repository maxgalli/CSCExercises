<?php

	echo "Movies with official webpages:";
	
	$movies = query_array("select * from movie where webpage <> ''");

	echo '<ul>';
	foreach ($movies as $m) {
		echo '<li><a href="'.$m["webpage"].'">'.$m["title"].'</a>';
	}
	echo '</ul>';
	
?>
