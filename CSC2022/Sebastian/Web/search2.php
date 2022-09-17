<?php

	$q = getArg("q");
	
	$any = false;
	if ($q != "") {
		$any = true;
		if ($q == "all") $q = '';
	}
			
	echo '
		<form action="index.php" method="GET">
		<input type="hidden" name="p" value="search2">
		<input type="text" size="80" name="q" value="'.$q.'">
		<input type="submit" value="Search">
		</form>';
	
	if ($any) {

		$query = "select id, title from movie where title like '%$q%'";
		echo "<p>$query</p>";
	
		echo "Search results:";
		$movies = query_array($query);
			
		if (count($movies) == 0) echo "<p>nothing found for '$q'";
		else {	
			echo '<ul>';
			foreach ($movies as $m) {
				echo '<li><a href="index.php?p=movie&id='.$m["id"].'">'.$m["title"].'</a>';
			}
			echo '</ul>';
		}
	}	
?>
