<?php

	$best = (getArg("q") == "best");

	if ($best) echo "Best"; 
	else echo "Worst";
	
	echo " movies (top 3) as ranked by users:";
	
	$movies = query_array(
		'select *, rating_sum/rating_count as rating ' .
		'from movie where rating_count > 0 order by rating '.($best?'desc':''));

	echo '<ul>';
	$n = min(3, count($movies));
	for ($i = 0; $i < $n; $i++) {
		$m = $movies[$i];
		echo '<li><a href="index.php?p=movie&id='.$m["id"].'">'.$m["title"].'</a> ('.
			$m["year"].') - '.$m["rating"].' / 10';
	}
	echo '</ul>';
	
?>
