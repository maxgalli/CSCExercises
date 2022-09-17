<?php

	$id = getArg("id");
	
	$rate = getArg("rate");
	if ($rate != "") {
		$res = execute_query(
		'update movie ' .
		'set rating_sum = rating_sum + '.$rate.', rating_count = rating_count + 1 ' .
		'where id = '.$id);
		if ($res) echo "(your rating of $rate was added!)<p>";
	}
	
	$comment = mysql_real_escape_string(getArg("comment"));
	if ($comment != "") {
		if (query_single(
			"select count(*) from comment where movie_id = $id and " .
			"comment = '$comment'") == 0)
				execute_query("insert into comment values ($id, '$comment')");
	}
	 	
	$m = query_row(
		'select *, rating_sum/rating_count as rating '.
		'from movie where id = '.$id);
		
	echo ' 
		<b>'.$m["title"].'</b> ('.$m["year"].')<p>
		Director: '.$m["director"].'<br>
		Starring: '.$m["stars"].'<p>';
	
	if ($m["webpage"] != "") echo '
		Webpage: <a href="'.$m["webpage"].'">'.$m["webpage"].'</a></p>';
	
	if ($m["rating_count"] != 0) echo '
		Rating: '.$m["rating"].' / 10 &nbsp;&nbsp;&nbsp;('.$m["rating_count"].' people voted)';
	else echo "not rated yet";

	
	echo '<p>Give your rating for this movie:<br>(horrible) ';
	for ($i = 1; $i <= 10; $i++) 
		echo '<a href="index.php?p=movie&id='.$id.'&rate='.$i.'">'.$i.'</a>&nbsp;&nbsp;&nbsp;';
	echo ' (great)'; 	
	
	echo '
		<hr size="1" align="left" width="300">
		<form action="index.php" method="GET">
		<input type="hidden" name="p" value="movie">
		<input type="hidden" name="id" value="'.$id.'">
		Add your comment:<br>
		<textarea name="comment" cols="50"></textarea><br>
		<input type="submit" value="Add this comment">		
		</form>
		
		Comments:
		<ul>';
		
	$comments = query_array("select comment from comment where movie_id = ".$id);
			
	if (count($comments) == 0) echo "No comments entered for this movie";
	else foreach ($comments as $c)
		echo '<li>'.$c["comment"];
		
	echo '</ul>';
?>
