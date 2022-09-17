<?php

function initDB($host, $user, $pwd, $dbname)
{
	global $db;
	$db = new mysqli($host, $user, $pwd, $dbname);
/*
	$db = mysql_connect($host, $user, $pwd)
  		or die('Could not connect: ' . mysql_error());
	mysql_select_db($dbname) or die('Could not select database');
*/
}

function mysql_real_escape_string($value)
{
        global $db;
	return $db->real_escape_string($value);
}

function execute_query($query)
{
        global $db;
	return $db->query($query) or die("Query failed: (" . $db->errno . ") " . $db->error);
/*
	if (!$db->query($query)) { 
		die("Query failed: (" . $db->errno . ") " . $db->error);
	}
*/
}

function query_array($query)
{
        global $db;
	$qr = $db->query($query);

        $res = array();
	$qr->data_seek(0);
	while ($row = $qr->fetch_assoc())
                $res[] = $row;

	return $res;
}

/*
function query_asoc_array($query)
{
	// TO TEST
	$qr = mysql_query($query) or die('Query failed: ' . mysql_error());
	
	$res = array();
	while ($row = mysql_fetch_array($qr)) 
		$res[$row[0]] = $row;
	
	mysql_free_result($qr);	 
	return $res;	 
}
*/

function query_single($query)
{
	$res = query_array($query);
	if (count($res) == 0) return null;
	else return array_values($res[0])[0];
}

function query_row($query)
{
	$res = query_array($query);
	if (count($res) == 0) return null;
	else return $res[0];
}



function closeDB()
{
	global $db;
//	mysql_close($db);
}
	
?>
