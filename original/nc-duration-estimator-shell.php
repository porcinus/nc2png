<?php
if(realpath(__FILE__)==realpath($_SERVER['DOCUMENT_ROOT'].$_SERVER['SCRIPT_NAME'])){$ncscript_is_included=false;}else{$ncscript_is_included=true;}

function NNS_NC_Settings(&$return_array){ //XXX: NNS_NC_Settings()
	date_default_timezone_set('Europe/Paris');

	$return_array['settings']['cnc']['speed_fast_XY'] = 2500; //rapide XY
	$return_array['settings']['cnc']['speed_fast_Z'] = 1000; //rapide Z
	$return_array['settings']['cnc']['speed_percent'] = 100; //pourcentage vitesse max
	
	$return_array['settings']['cnc']['speed_fast_XY_reset'] = $return_array['settings']['cnc']['speed_fast_XY']; //rapide XY
	$return_array['settings']['cnc']['speed_fast_Z_reset'] = $return_array['settings']['cnc']['speed_fast_Z']; //rapide Z
	$return_array['settings']['cnc']['speed_percent_reset'] = $return_array['settings']['cnc']['speed_percent']; //pourcentage vitesse max
	
	$return_array['settings']['general']['target_dir'] = "tmp/"; //chein des fichier nc uploader
	$return_array['settings']['general']['cache_duration'] = 60*15; //date limite de cache pour suppression
	
	$return_array['settings']['gd']['ignore_limits'] = false;
	$return_array['settings']['gd']['ignore_tools'] = false;
	$return_array['settings']['gd']['ignore_work'] = false;
	$return_array['settings']['gd']['ignore_fast'] = false;
	
	$return_array['settings']['gd']['width'] = 800; //px
	$return_array['settings']['gd']['margin'] = 10; //mm
	$return_array['settings']['gd']['grid'] = 10; //mm
	$return_array['settings']['gd']['subgrid'] = 1; //mm
	$return_array['settings']['gd']['subgrid_minpx'] = 5; //px
	$return_array['settings']['gd']['arc_resolution'] = 10; //px
	$return_array['settings']['gd']['axis_fontsize'] = 5;
	$return_array['settings']['gd']['axis_fontwidth'] = imagefontwidth($return_array['settings']['gd']['axis_fontsize']);
	$return_array['settings']['gd']['axis_fontheight'] = imagefontheight($return_array['settings']['gd']['axis_fontsize']);
	
	$return_array['settings']['gd']['str_fontsize'] = 2;
	$return_array['settings']['gd']['str_fontwidth'] = imagefontwidth($return_array['settings']['gd']['str_fontsize']);
	$return_array['settings']['gd']['str_fontheight'] = imagefontheight($return_array['settings']['gd']['str_fontsize']);
	
	$return_array['settings']['gd']['str_grid'] = "Grille : {$return_array['settings']['gd']['grid']}mm";
	$return_array['settings']['gd']['str_axis'] = "Axes";
	$return_array['settings']['gd']['str_tool'] = "Chemin Outils";
	$return_array['settings']['gd']['str_fast'] = "Rapide";
	$return_array['settings']['gd']['str_work'] = "Interpolation Lineaire";
	$return_array['settings']['gd']['str_radius'] = "Interpolation Circulaire";
	$return_array['settings']['gd']['str_time'] = "Temps Total Theorique";
	$return_array['settings']['gd']['str_timepercent'] = "Temps Total Theorique";
	
	$return_array['settings']['gd']['color_background'] = array(0,0,0);
	$return_array['settings']['gd']['color_grid'] = array(70,70,70);
	$return_array['settings']['gd']['color_subgrid'] = array(35,35,35);
	$return_array['settings']['gd']['color_axis'] = array(180,180,180);
	$return_array['settings']['gd']['color_toolpath_higher'] = array(0,200,200);
	$return_array['settings']['gd']['color_toolpath_lower'] = array(0,80,80);
	$return_array['settings']['gd']['color_fast'] = array(255,0,0);
	$return_array['settings']['gd']['color_linear'] = array(0,0,255);
	$return_array['settings']['gd']['color_circular'] = array(0,255,0);
	$return_array['settings']['gd']['color_limits'] = array(255,255,0);
	$return_array['settings']['gd']['color_limitslines'] = array(180,180,0);
	
	global $argv;if(isset($argv)){$return_array['settings']['general']['run_from_cmd']=true;$return_array['settings']['general']['query']=implode(' ',array_slice($argv,1));unset($argv[0]);}else{$return_array['settings']['general']['run_from_cmd']=false;if(count($_POST)>0){$return_array['settings']['general']['query']=$_POST;}else{$return_array['settings']['general']['query']=false;}}
	$return_array['fatal_error'] = false; //erreur fatale : false : tout va bien
	
	$return_array['info']['compute_time']['unit'] = 'sec';
	$return_array['info']['memorypeak']['unit'] = 'kb';
	
	return true;
}

function NNS_fileexists_pathvar($program_name){$path=@getenv('PATH');if($path===false){return false;}if(strtoupper(substr(PHP_OS,0,3))==='WIN'){$pathlimiter='\\';$is_win=true;}else{$pathlimiter='/';$is_win=false;}$path.=';';$path=explode(';',$path,-1);$pathcount=count($path);for($i=0;$i<$pathcount;$i++){$temppath = $path[$i];if(substr($temppath,-1)!==$pathlimiter){$temppath.=$pathlimiter;}if(file_exists($temppath.$program_name)){return true;}if($is_win){if(file_exists($temppath.$program_name.'.exe')){return true;}}}return false;}
function NNS_FlushFolder($folder,$timelimit){$deletemtime = time()-($timelimit);$tmpfileslist = scandir($folder);for($i=0;$i<count($tmpfileslist);$i++){if(is_file($folder.$tmpfileslist[$i]) && @filemtime($folder.$tmpfileslist[$i])<$deletemtime){unlink($folder.$tmpfileslist[$i]);}}return true;}
function NNS_numberdiff($n1,$n2){if($n1>$n2){return abs($n2-$n1);}else{return abs($n1-$n2);}}
function NNS_sec2str($sec=0){$sec=floor($sec);if($sec<60){return $sec.'sec';}else{if($sec<3600){$minutes=ceil($sec/60);if($minutes<60){return $minutes.'min';}}$hours=floor($sec/3600);$minutes=ceil(($sec%3600)/60);if($minutes>59){$minutes=0;$hours++;}return sprintf('%02dh %02dmin',$hours,$minutes);}} //fonction de parse de temps : secondes vers x
function NNS_angle3point($cx,$cy,$x1,$y1,$x2,$y2){$v1x=$x2-$cx;$v1y=$y2-$cy;$v2x=$x1-$cx;$v2y=$y1-$cy;return acos(($v1x*$v2x+$v1y*$v2y)/(sqrt($v1x*$v1x+$v1y*$v1y)*sqrt($v2x*$v2x+$v2y*$v2y)));} //fonction de calcul d'angle
function NNS_imagelinethick($image,$x1,$y1,$x2,$y2,$thickness,$color){if($thickness==1){return imageline($image,$x1,$y1,$x2,$y2,$color);}imagefilledellipse($image,$x1,$y1,$thickness,$thickness,$color);imagefilledellipse($image,$x2,$y2,$thickness,$thickness,$color);$halfthick=$thickness/2;if($x1==$x2){return imagefilledrectangle($image,$x1-$halfthick,$y1,$x2+$halfthick,$y2,$color);}elseif($y1==$y2){return imagefilledrectangle($image,$x1,$y1-$halfthick,$x2,$y2+$halfthick,$color);}else{$angle=atan(($y2-$y1)/($x2-$x1));$deccos=$halfthick*cos($angle);$decsin=$halfthick*sin($angle);return imagefilledpolygon($image,array(round($x1+$decsin),round($y1-$deccos),round($x1-$decsin),round($y1+$deccos),round($x2-$decsin),round($y2+$deccos),round($x2+$decsin),round($y2-$deccos)),4,$color);}}
function NNS_imagearcthick($image,$cx,$cy,$width,$height,$start,$end,$thickness,$color,$resolution=20){if($resolution<1){$resolution=20;}else{$resolution=ceil($resolution);}if($thickness==1){return imagearc($image,$cx,$cy,$width,$height,$start,$end,$color);}$hwidth=$width/2;$hheight=$height/2;$a1=deg2rad($start);$a2=deg2rad($end);imagefilledellipse($image,round($cx+cos($a1)*$hwidth),round($cy+sin($a1)*$hheight),$thickness,$thickness,$color);imagefilledellipse($image,round($cx+cos($a2)*$hwidth),round($cy+sin($a2)*$hheight),$thickness,$thickness,$color);$arclenght = abs($a2-$a1)*($height/2);$step = ceil($arclenght/$resolution)+1;$stepinv = $step*4+2;$stepangle = ($a2-$a1)/$step;$radiusxmin = $hwidth-$thickness/2;$radiusymin = $hheight-$thickness/2;$radiusxmax = $hwidth+$thickness/2;$radiusymax = $hheight+$thickness/2;for($i=0,$j=0;$i<($step+1)*2;$i+=2,$j++){$tmpx=cos($a1+$stepangle*$j);$tmpy=sin($a1+$stepangle*$j);$tmparray[$i]=$cx+$tmpx*$radiusxmin;$tmparray[$i+1]=$cy+$tmpy*$radiusymin;$tmparray[$stepinv-$i]=$cx+$tmpx*$radiusxmax;$tmparray[$stepinv-$i+1]=$cy+$tmpy*$radiusymax;}return imagefilledpolygon($image,$tmparray,($step+1)*2,$color);}
function NNS_imagelinethickV($image,$x1,$y1,$x2,$y2,$d1,$d2,$color){if($x2-$x1<=0){$tmpx=$x1;$tmpy=$y1;$tmpd=$d1;$x1=$x2;$y1=$y2;$x2=$tmpx;$y2=$tmpy;$d1=$d2;$d2=$tmpd;}imagefilledellipse($image,$x1,$y1,$d1,$d1,$color);imagefilledellipse($image,$x2,$y2,$d2,$d2,$color);$hd1=$d1/2;$hd2=$d2/2;if($y2==$y1){$y2++;}if($x2==$x1){$x2++;}$vx=$x2-$x1;$vy=$y2-$y1;$centerdist=sqrt($vx*$vx+$vy*$vy);$relangle=acos(($hd1-$hd2)/$centerdist);$absangle=atan($vy/$vx);$anglea=$relangle-$absangle;$angleb=$relangle+$absangle;$sina=sin($anglea);$sinb=sin($angleb);$cosa=cos($anglea);$cosb=cos($angleb);return imagefilledpolygon($image,array(round($x1+$hd1*$cosa),round($y1-$hd1*$sina),round($x2+$hd2*$cosa),round($y2-$hd2*$sina),round($x2+$hd2*$cosb),round($y2+$hd2*$sinb),round($x1+$hd1*$cosb),round($y1+$hd1*$sinb)),4,$color);}
function NNS_imagearcthickV($image,$cx,$cy,$width,$height,$start,$end,$d1,$d2,$color,$resolution=30){if($resolution<1){$resolution=20;}else{$resolution=ceil($resolution);}if($d1==1){return imagearc($image,$cx,$cy,$width,$height,$start,$end,$color);}$hwidth=$width/2; $hheight=$height/2;$a1=deg2rad($start); $a2=deg2rad($end);imagefilledellipse($image,round($cx+cos($a1)*$hwidth),round($cy+sin($a1)*$hheight),$d1,$d1,$color);imagefilledellipse($image,round($cx+cos($a2)*$hwidth),round($cy+sin($a2)*$hheight),$d2,$d2,$color);$arclenght = ($a2-$a1)*($height/2);$step = ceil($arclenght/$resolution);$stepinv = $step*4+2;$stepangle = ($a2-$a1)/$step;$stepd=($d2-$d1)/$step;for($i=0,$j=0;$i<($step+1)*2;$i+=2,$j++){$tmpx=cos($a1+$stepangle*$j);$tmpy=sin($a1+$stepangle*$j);$radiusxmin = $hwidth-($d1+$stepd*($j))/2;$radiusymin = $hheight-($d1+$stepd*($j))/2;$radiusxmax = $hwidth+($d1+$stepd*($j))/2;$radiusymax = $hheight+($d1+$stepd*($j))/2;$tmparray[$i]=$cx+$tmpx*$radiusxmin;$tmparray[$i+1]=$cy+$tmpy*$radiusymin;$tmparray[$stepinv-$i]=$cx+$tmpx*$radiusxmax;$tmparray[$stepinv-$i+1]=$cy+$tmpy*$radiusymax;}return imagefilledpolygon($image,$tmparray,($step+1)*2,$color);}
function NNS_imagearrow($image,$x,$y,$lenght,$width,$height,$dir,$color,$filled=false){if($dir=='x+'){imageline($image,$x,$y,$x+$lenght,$y,$color);$polyarray=array($x+$lenght-$width,$y-$height/2,$x+$lenght,$y,$x+$lenght-$width,$y+$height/2);}elseif($dir=='x-'){imageline($image,$x,$y,$x-$lenght,$y,$color);$polyarray=array($x-$lenght+$width,$y-$height/2,$x-$lenght,$y,$x-$lenght+$width,$y+$height/2);}elseif($dir=='y+'){imageline($image,$x,$y,$x,$y+$lenght,$color);$polyarray=array($x-$height/2,$y+$lenght-$width,$x,$y+$lenght,$x+$height/2,$y+$lenght-$width);}elseif($dir=='y-'){imageline($image,$x,$y,$x,$y-$lenght,$color);$polyarray=array($x-$height/2,$y-$lenght+$width,$x,$y-$lenght,$x+$height/2,$y-$lenght+$width);}else{return false;}if($filled){imagefilledpolygon($image,$polyarray,3,$color);}else{imagepolygon($image,$polyarray,3,$color);}return true;}
function NNS_imagearc($image,$cx,$cy,$width,$height,$start,$end,$color,$resolution=30){if($resolution<1){$resolution=20;}else{$resolution=ceil($resolution);}$hwidth=$width/2;$hheight=$height/2;$a1=deg2rad($start);$a2=deg2rad($end);$arclenght=($a2-$a1)*($height/2);$step=ceil($arclenght/$resolution);$stepangle=($a2-$a1)/$step;$lastx=$cx+cos($a1)*$hwidth;$lasty=$cy+sin($a1)*$hheight;for($i=1;$i<($step+1);$i++){$x=$cx+cos($a1+$stepangle*$i)*$hwidth;$y=$cy+sin($a1+$stepangle*$i)*$hheight;imageline($image,$x,$y,$lastx,$lasty,$color);$lastx=$x;$lasty=$y;}}
function NNS_arclimits($cx,$cy,$width,$height,$start,$end,$resolution=1){if($resolution<1){$resolution=20;}else{$resolution=ceil($resolution);}$hwidth=$width/2;$hheight=$height/2;$a1=deg2rad($start);$x1=$cx+cos($a1)*$hwidth;$y1=$cy-sin($a1)*$hheight;$a2=deg2rad($end);$x2=$cx+cos($a2)*$hwidth;$y2=$cy-sin($a2)*$hheight;if($x1>$x2){$xmin=$x2;$xmax=$x1;}else{$xmin=$x1;$xmax=$x2;}if($y1>$y2){$ymin=$y2;$ymax=$y1;}else{$ymin=$y1;$ymax=$y2;}$arclenght=($a2-$a1)*($height/2);$step=ceil($arclenght/$resolution);$stepangle=($a2-$a1)/$step;for($i=1;$i<($step+1);$i++){$x=$cx+cos($a1+$stepangle*$i)*$hwidth;$y=$cy-sin($a1+$stepangle*$i)*$hheight;if($x<$xmin){$xmin=$x;}if($x>$xmax){$xmax=$x;}if($y<$ymin){$ymin=$y;}if($y>$ymax){$ymax=$y;}}return array('x-'=>$xmin,'x+'=>$xmax,'y-'=>$ymin,'y+'=>$ymax);}
function NNS_imagelinedashed($image,$x1,$y1,$x2,$y2,$color1,$color2=IMG_COLOR_TRANSPARENT,$space=4){for($i=0;$i<$space;$i++){$style[]=$color1;}for($i=0;$i<$space;$i++){$style[]=$color2;}imagesetstyle($image,$style);return imageline($image,$x1,$y1,$x2,$y2,IMG_COLOR_STYLED);}
function NNS_imagecolorallocatefade($im,$RGBarray1,$RGBarray2,$fade=0){if($fade<=0){return imagecolorallocate($im,$RGBarray1[0],$RGBarray1[1],$RGBarray1[2]);} if($fade>=1){return imagecolorallocate($im,$RGBarray2[0],$RGBarray2[1],$RGBarray2[2]);} return imagecolorallocate($im,$RGBarray1[0]+($RGBarray2[0]-$RGBarray1[0])*$fade,$RGBarray1[1]+($RGBarray2[1]-$RGBarray1[1])*$fade,$RGBarray1[2]+($RGBarray2[2]-$RGBarray1[2])*$fade);}

function NNS_NC_OpenFile($ncfile,&$return_array,$restrict=array('nc','code','numcode','txt')){ //XXX: NNS_NC_OpenFile()
	if($return_array['fatal_error']){return false;}
	$computetime = microtime(true);
	$ncfile_ext = strtolower(end(explode('.',$ncfile))); //extension du fichier
	$tmpncfile = strtolower(end(explode('/',str_replace('\\','/',$ncfile)))); //nom du fichier seul
	
	if(!in_array($ncfile_ext,$restrict)){
		if(!$return_array['settings']['general']['run_from_cmd'] && strpos($ncfile,$return_array['settings']['general']['target_dir'])!==false && @is_file($ncfile)){@unlink($ncfile);}
		$return_array['fatal_error'] = "Format de fichier accepte : ".implode(' , ',$restrict);
		return false;
	} //si extension non valide
	
	if(file_exists($ncfile) && @is_file($ncfile) && @is_readable($ncfile)){ //si le fichier existe et pas d'erreur fatale : lecture du fichier
		$ncfile_content = file_get_contents($ncfile); //lecture du fichier
		$ncfile_content = str_replace("\xEF\xBB\xBF",'',$ncfile_content); //strip utf
		$ncfile_content = str_replace("\r\n","\n",$ncfile_content); //win->unix
		$ncfile_content = str_replace('  ',' ',$ncfile_content); //win->unix
		$ncfile_content = strtolower($ncfile_content); //lowercase
		$ncfile_content = preg_replace('/g0(\d)/', 'g$1',$ncfile_content,-1); //suppression 0 des G
		$ncfile_content = preg_replace('/m0(\d)/', 'm$1',$ncfile_content,-1); //suppression 0 des M
		$ncfile_lenght = strlen($ncfile_content); //longueur du fichier
		$ncfile_content = explode("\n",$ncfile_content); //convertion programme en tableau
	}else{$return_array['fatal_error'] = "Echec lors de l'ouverture du fichier : '$ncfile'";return false;} //erreur de lecture : erreur fatale
	
	$return_array['file'] = $ncfile_content; //tableau du programme
	$return_array['info']['file']['filename'] = $tmpncfile;
	$return_array['info']['file']['fullpath'] = $ncfile;
	$return_array['info']['file']['ext'] = $ncfile_ext; //extension fichier
	$return_array['info']['file']['filesize'] = $ncfile_lenght; //longueur du fichier
	$return_array['info']['compute_time']['openfile'] = microtime(true)-$computetime;$return_array['info']['memorypeak']['openfile'] = memory_get_peak_usage(true)/1024;
	return true;
}


function NNS_NC_ExtractData(&$return_array,$include_preview=true){ //XXX: NNS_NC_ExtractData()
	if($return_array['fatal_error']){return false;}
	$computetime = microtime(true);
	if(!is_array($return_array)){$return_array['fatal_error'] = "Echec lors de l'extration des données";return false;} //$return_array n'est pas un tableau : erreur fatale
	if(!isset($return_array['file'])){$return_array['fatal_error'] = "Le tableau ne contient pas de données";return false;} //$return_array['file'] n'existe pas : erreur fatale
	
	$ncfile_content = $return_array['file']; $lines_count = count($ncfile_content);
	$speed_fast_XY = $return_array['settings']['cnc']['speed_fast_XY']; $speed_fast_Z = $return_array['settings']['cnc']['speed_fast_Z']; $speed_percent = $return_array['settings']['cnc']['speed_percent'];
	$minX = 0; $minY = 0; $minZ = 0; $maxX = 0; $maxY = 0; $maxZ = 0; //positions absolu de nc
	$minZ_work = 0; $maxZ_work = 0; //positions absolu de nc en Z travail
	$firstX = true; $firstY = true; $firstZ = true; //premiere detection des limites 
	$time_fast = $time_work = 0; //temps en rapide, travail
	$lines_comment = $lines_ignored = 0; //nombre de ligne commentaires, ignorées
	$lines_g0 = $lines_g1 = $lines_g2 = $lines_g3 = $lines_drill = 0; //nombre de ligne en G0 (rapide), G1 (lineaire), G2 et G3 (circulaire), drill : G81 et G83
	$travel_radial = $travel_real = $travel_drill = $travel_fast_real = 0; //distance parcours en interpolation circulaire en travail, reel en travail, reel en cyle de percage, reel en rapide
	$new_x = $last_x = $new_y = $last_y = $new_z = $last_z = 0; $new_r=false; //derniere position + definition pour premiere boucle : position actuel
	$tooldia = 0; $tools[] = 0; $toolused[] = 0; $nctools[] = 0; //outil en cours
	$last_speed = $speed_fast_XY; $new_speed = $last_speed; //derniere vitesse d'avance + definition pour premiere boucle : vitesse d'avance actuel
	$realline = 0; //position ligne reel
	$nc_absolute = true; //programme codé en absolue ? si false -> relatif
	$tmptype = 'fast'; /*$lineiscomment = false;*/ //type de travail en cours
	$comment_time = array(); //dernier commentaire pour rapport temps + tableau commentaire pour rapport temps
	
	for($i=0;$i<$lines_count;$i++){ //boucle de lecture
		$realline++; //increment position de ligne réel
		$tmpline = $ncfile_content[$i]; //variable contenant la ligne en cours
		
		if(strpos($tmpline,'(')!==false && strpos($tmpline,')')!==false){ //si ligne commentaire
			$lines_comment++; //increment compteur de commentaire
			$tmpline=substr($tmpline,strpos($tmpline,'(')+1,-(strlen($tmpline)-strrpos($tmpline,')'))); //cleanup ligne commentaire
			$comment_time[$lines_comment]['comment'] = $tmpline; $comment_time[$lines_comment]['time_fast'] = 0; $comment_time[$lines_comment]['time_work'] = 0; $comment_time[$lines_comment]['absolute'] = $nc_absolute; //tableau
			
			if(strpos($tmpline,'tool/mill')!==false){ //detection outil cutview
				$tmpcomment=explode(',',$tmpline); $tooldia=trim($tmpcomment[1]); $toolangle=trim($tmpcomment[4]); //definition pour la suite du script
				if(!in_array($tooldia,$tools)){$tools[]=array('d'=>$tooldia,'d_unit'=>'mm','v'=>$toolangle,'v_unit'=>chr(176));} //si non detecté precedement, ajout au tableau d'outils
				$toolused[$tooldia.'v'.$toolangle]=0;
			}
			
			if(preg_match('/t([0-9]+)\s?:\s?([0-9]+\.?([0-9]+)?)/i',$tmpline,$tmpmatches)){ //detection outil T cutview // ( T10 : 2.999 )
				if(isset($tmpmatches[1])&&isset($tmpmatches[2])){$nctools[$tmpmatches[2]]=$tmpmatches[1];} //si Tn et dia detecté, ajout au tableau d'outils Num
			}
			
			if(strpos($tmpline,'stock/block')!==false){ //detection block cutview
				$tmpcomment=explode(',',$tmpline);
				$stock_ox=$tmpcomment[4]*-1;$stock_oy=$tmpcomment[5]*-1;$stock_lx=$tmpcomment[1]+$stock_ox;$stock_ly=$tmpcomment[2]+$stock_oy;
				if($stock_ox!==$stock_lx && $stock_oy!==$stock_ly){$minX=$stock_ox; $minY=$stock_oy;$maxX=$stock_lx; $maxY=$stock_ly;$firstX = false; $firstY = false;}
			}
			
			continue; //skip de la boucle
		}else{ //si la ligne n'est pas un commentaire
			$tmpline = preg_replace('/([a-z])/', ' $1',$tmpline,-1); //cleanup de la ligne
			$tmplinecnt = explode(' ',$tmpline); //convertion en tableau pour simplifier le parse de la ligne
			$tmpelement_count = count($tmplinecnt); //nombre d'element détecté
			$nothinginline = true; $new_i=false; $new_j=false; /*$new_k=false;*/ $new_q=false; /*$new_r=false*/; //reset de certaine valeur
			for($j=0;$j<$tmpelement_count;$j++){ //boucle de lecture des elements
				$tmplinecntid = substr($tmplinecnt[$j],0,1); //fastup
				if($tmplinecnt[$j]==''){ //fastup
				}elseif($tmplinecntid=='g'){ //si l'element commence par G
					if($tmplinecnt[$j]=='g90'){$nc_absolute = true; //passage en deplacement absolue
					}elseif($tmplinecnt[$j]=='g91'){$nc_absolute = false; //passage en deplacement relative
					}elseif($tmplinecnt[$j]=='g0'){$tmptype = 'fast'; //passage en rapide
					}elseif($tmplinecnt[$j]=='g1' || $tmplinecnt[$j]=='g2' || $tmplinecnt[$j]=='g3'){$tmptype='work'; //passage en travail
					}elseif($tmplinecnt[$j]=='g81' || $tmplinecnt[$j]=='g83'){$tmptype = 'drill'; //cycle de percage
					}else{$tmptype='other';}
					$tmp_op = 'g'.substr($tmplinecnt[$j],1); $nothinginline = false; //definition nouvelle valeur
				}elseif($tmplinecntid=='f'){ //si l'element commence par F
					$new_speed = round(substr($tmplinecnt[$j],1)); $nothinginline = false; //definition nouvelle valeur
					if($new_speed>$speed_fast_XY){$new_speed=$speed_fast_XY;} //overide si > rapide
				}elseif($tmplinecntid=='x'){ //si l'element commence par X
					$new_x = round(substr($tmplinecnt[$j],1),4); $nothinginline = false; //definition nouvelle valeur
					if($firstX){$minX=$new_x;$maxX=$new_x;$firstX=false;}
					if($new_x<$minX){$minX=$new_x;} if($new_x>$maxX){$maxX=$new_x;}
				}elseif($tmplinecntid=='y'){ //si l'element commence par Y
					$new_y = round(substr($tmplinecnt[$j],1),4); $nothinginline = false; //definition nouvelle valeur
					if($firstY){$minY=$new_y;$maxY=$new_y;$firstY=false;}
					if($new_y<$minY){$minY=$new_y;} if($new_y>$maxY){$maxY=$new_y;}
				}elseif($tmplinecntid=='z'){ //si l'element commence par Z
					$new_z = round(substr($tmplinecnt[$j],1),4); $nothinginline = false; //definition nouvelle valeur
					if($firstZ){$minZ=$new_z;$maxZ=$new_z;$firstZ=false;}
					if($new_z<$minZ){
						$minZ=$new_z;
						if($tmptype!='fast'){$minZ_work=$new_z;} //si en mode travail
					}
					
					if($new_z>$maxZ){
						$maxZ=$new_z;
						if($tmptype!='fast'){$maxZ_work=$new_z;} //si en mode travail
					}
					
				}elseif($tmplinecntid=='i'){ //si l'element commence par I : centre X
					$new_i = round(substr($tmplinecnt[$j],1),4); $nothinginline = false; //definition nouvelle valeur
				}elseif($tmplinecntid=='j'){ //si l'element commence par J : centre Y
					$new_j = round(substr($tmplinecnt[$j],1),4); $nothinginline = false; //definition nouvelle valeur
				}elseif($tmplinecntid=='k'){ //si l'element commence par K : centre Z (préimplement pour evolution code)
					//$new_k = round(substr($tmplinecnt[$j],1),4); $nothinginline = false; //definition nouvelle valeur (préimplement pour evolution code)
				}elseif($tmplinecntid=='q'){ //si l'element commence par Q : cycle percage : procondeur de débourage (inversé)
					$new_q = round(substr($tmplinecnt[$j],1),4); $nothinginline = false; //definition nouvelle valeur
				}elseif($tmplinecntid=='r'){ //si l'element commence par R : cycle percage : plan de remonté
					$new_r = round(substr($tmplinecnt[$j],1),4); $nothinginline = false; //definition nouvelle valeur
				}
			}
			
			if(!$nothinginline && ($tmp_op=='g2' || $tmp_op=='g3')){  //------------------------------si interpolation circulaire G2 ou G3
				if($tmp_op=='g2'){$lines_g2++;$angledir=1;}elseif($tmp_op=='g3'){$lines_g3++;$angledir=-1;} //pointage ligne pour rapport
				
				if($new_i!==false && $new_j!==false && (($new_x != $last_x) || ($new_y != $last_y) || ($new_z != $last_z))){ //------------------------------------si centre I et J défini
					$new_r = abs(round(sqrt(NNS_numberdiff($new_i,$new_x)*NNS_numberdiff($new_i,$new_x)+NNS_numberdiff($new_j,$new_y)*NNS_numberdiff($new_j,$new_y)),4)); //calcule du rayon
					
					$new_total_angle = round(rad2deg(NNS_angle3point($new_i,$new_j,$last_x,$last_y,$new_x,$new_y)),2);
					$new_total_anglestart = round(rad2deg(NNS_angle3point($new_i,$new_j,$new_i+$new_r,$new_j,$last_x,$last_y,$angledir)),2);
					$new_total_angleend = round(rad2deg(NNS_angle3point($new_i,$new_j,$new_i+$new_r,$new_j,$new_x,$new_y,$angledir)),2);
					
					if($tmp_op=='g3'){$new_total_angletmp=$new_total_anglestart;$new_total_anglestart=$new_total_angleend;$new_total_angleend=$new_total_angletmp;}
					
					if(($last_y>$new_j && $angledir>0) || ($new_y>$new_j && $angledir<0)){
						if(($last_x>$new_i && $angledir>0) || ($new_x>$new_i && $angledir<0)){$new_total_anglestart=360-$new_total_anglestart;
						}else{$new_total_anglestart=180-$new_total_anglestart+180;}
					}
					
					if(($new_y>$new_j && $angledir>0) || ($last_y-$new_j>0 && $angledir<0)){
						if(($new_x>$new_i && $angledir>0) || ($last_x>$new_i && $angledir<0)){$new_total_angleend=360-$new_total_angleend;
						}else{$new_total_angleend=180-$new_total_angleend+180;}
					}
					
					if($new_total_anglestart==360){$new_total_anglestart=0;}
					if($new_total_angleend==0 && $new_total_anglestart>0){$new_total_angleend=360;}
					
					if($new_total_anglestart+$new_total_angle>360 || $new_total_anglestart>$new_total_angleend){
						$arclimits = NNS_arclimits($new_i,$new_j,$new_r*2,$new_r*2,$new_total_anglestart,360);
						if($arclimits['x-']<$minX){$minX=$arclimits['x-'];} if($arclimits['x+']>$maxX){$maxX=$arclimits['x+'];} if($arclimits['y-']<$minY){$minY=$arclimits['y-'];} if($arclimits['y+']>$maxY){$maxY=$arclimits['y+'];}
						
						$toolused[$tooldia.'v'.$toolangle]++;
						if($include_preview){$preview_array[] = array('type'=>$tmp_op, 'tool'=>$tooldia, 'toolangle'=>$toolangle, 'x'=>$new_x, 'y'=>$new_y, 'z'=>$new_z, 'i'=>$new_i, 'j'=>$new_j, 'startangle'=>$new_total_anglestart, 'endangle'=>360, 'r'=>$new_r);}
						
						$new_total_angle = (360-$new_total_anglestart)+$new_total_angleend;
						$new_total_anglestart = 0;
					}//donnee pour preview gd
					
					if($new_total_anglestart==0&&$new_total_angleend==360){
						$new_total_angleend=0;
					}
					
					if($new_total_anglestart!=$new_total_angleend){
						$arclimits = NNS_arclimits($new_i,$new_j,$new_r*2,$new_r*2,$new_total_anglestart,$new_total_angleend);
						if($arclimits['x-']<$minX){$minX=$arclimits['x-'];} if($arclimits['x+']>$maxX){$maxX=$arclimits['x+'];} if($arclimits['y-']<$minY){$minY=$arclimits['y-'];} if($arclimits['y+']>$maxY){$maxY=$arclimits['y+'];}
						
						$toolused[$tooldia.'v'.$toolangle]++;
						if($include_preview){$preview_array[] = array('type'=>$tmp_op, 'tool'=>$tooldia, 'toolangle'=>$toolangle, 'x'=>$new_x, 'y'=>$new_y, 'z'=>$new_z, 'i'=>$new_i, 'j'=>$new_j, 'startangle'=>$new_total_anglestart, 'endangle'=>$new_total_angleend, 'r'=>$new_r);}
					} //donnee pour preview gd
					
					if(!is_nan($new_total_angle)){ //si pas d'erreur lors du calcul d'angle
						$tmp_travel_tool = $new_r*deg2rad($new_total_angle); //calcul de la distance de travail
						if(($new_z != $last_z)){ //si deplacement Z
							if($nc_absolute){$tmp_travel_z = NNS_numberdiff($last_z,$new_z);}else{$tmp_travel_z = abs($new_z);} //absolue ou relatif (préimplement pour evolution code)
							$tmp_travel_tool = sqrt($tmp_travel_tool*$tmp_travel_tool+$tmp_travel_z*$tmp_travel_z); //calcul de la distance de travail via pythagore
						}
						$travel_real += $tmp_travel_tool; //mise a jour de la distance de travail total
						$time_work += $tmp_travel_tool/$new_speed; //mise a jour du temps de travail total
						$tmp_travel_tool = round($tmp_travel_tool,3); //arrondie de la distance de travail
						$travel_radial += $tmp_travel_tool; //mise a jour de la distance de travail total en interpolation circulaire
						$comment_time[$lines_comment]['time_work'] += $tmp_travel_tool/$new_speed; //mise a jour de la distance de travail total du commentaire
					}
					
					unset($tmp_travel_tool,$arclimits); //------------------------------------free de la distance calculé
				}
				
				$last_x = $new_x; $last_y = $new_y; $last_z = $new_z; //définition du nouveau XYZ pour prochaine boucle
				$last_speed = $new_speed; //définition du nouveau F pour prochaine boucle
			}elseif($tmptype=='drill'){ //---------------------------------------------------------------------Cycle de percage
				$lines_drill++; //pointage ligne pour rapport
				if(($new_x != $last_x)){if($nc_absolute){$tmp_travel_x = NNS_numberdiff($last_x,$new_x);}else{$tmp_travel_x = abs($new_x);}} //si nouveau X, calcule distance X en absolue ou relatif
				
				if(($new_y != $last_y)){if($nc_absolute){$tmp_travel_y = NNS_numberdiff($last_y,$new_y);}else{$tmp_travel_y = abs($new_y);}} //si nouveau Y, calcule distance Y en absolue ou relatif
				
				if(($new_x != $last_x) && ($new_y == $last_y)){ //deplacement seulement X
					$tmp_travel_tool = $tmp_travel_x; //deplacement d'outil sur l'axe X
				}elseif(($new_x == $last_x) && ($new_y != $last_y)){ //deplacement seulement Y
					$tmp_travel_tool = $tmp_travel_y; //deplacement d'outil sur l'axe Y
				}elseif(($new_x != $last_x) && ($new_y != $last_y)){ //deplacement X et Y
					$tmp_travel_tool = sqrt($tmp_travel_x*$tmp_travel_x+$tmp_travel_y*$tmp_travel_y); //deplacement d'outil sur l'axe X et Y : diagonale
				}
				
				$tmp_depth = NNS_numberdiff($new_z,$new_r); //profondeur totale
				if($new_q!=0){ //G83 : avec debourage
					$tmp_step = ceil($tmp_depth/$new_q); //nombre d'etapes
					$tmp_travel_z = 0; //reset de la distance de travail Z
					for($k=1;$k<$tmp_step+1;$k++){ //boucle pour chaque etape
						if($k==$tmp_step){ //si dernier etape
							$tmp_travel_z += $tmp_depth*2; //(profondeur totale)-(etape deja effectué)
						}else{ //si autre etape
							$tmp_travel_z += $new_q*$k*2; //(etape en cours)
						}
					}
				}else{ //G81 : sans debourage
					$tmp_travel_z = $tmp_depth*2; //(profondeur totale)-(etape deja effectué)
				}
				
				$tmp_travel_tool += $tmp_travel_z; //deplacement d'outil sur l'axe Z
				$travel_drill += $tmp_travel_z; //mise a jour de la distance de travail en cycle de perçage
				
				$toolused[$tooldia.'v'.$toolangle]++;
				if($include_preview){$preview_array[] = array('type'=>$tmp_op, 'tool'=>$tooldia, 'toolangle'=>$toolangle, 'x'=>$new_x, 'y'=>$new_y ,'z'=>$new_z);} //donnee pour preview gd
				
				if(isset($tmp_travel_tool)){ //si la distance calculé
					$travel_real += $tmp_travel_tool; //mise a jour de la distance de travail total
					$time_work += ($tmp_travel_tool/$new_speed); //mise a jour du temps de travail total
					$comment_time[$lines_comment]['time_work'] += ($tmp_travel_tool/$new_speed); //mise a jour du temps de travail total du commentaire
				}
				
				unset($tmp_travel_tool); //-----------------------------------free de la distance calculé
				$last_x = $new_x; $last_y = $new_y; $last_z = $new_z; //définition du nouveau XYZ pour prochaine boucle
				$last_speed = $new_speed; //définition du nouveau F pour prochaine boucle
			}elseif(!$nothinginline){  //----------------------------------------------------------------------G0 ou G1 : OK?
				if($tmptype=='work'){$lines_g1++;}elseif($tmptype=='fast'){$lines_g0++;} //pointage ligne pour rapport
				
				if(($new_x != $last_x)){ //si nouveau X
					if($nc_absolute){$tmp_travel_x = NNS_numberdiff($last_x,$new_x);}else{$tmp_travel_x = abs($new_x);} //calcule distance X en absolue ou relatif
					if($tmptype=='fast'){$new_speed = $speed_fast_XY;} //mise a jour de la distance de travail ou rapide axe X total
				}
				
				if(($new_y != $last_y)){ //si nouveau Y
					if($nc_absolute){$tmp_travel_y = NNS_numberdiff($last_y,$new_y);}else{$tmp_travel_y = abs($new_y);} //calcule distance Y en absolue ou relatif
					if($tmptype=='fast'){$new_speed = $speed_fast_XY;} //mise a jour de la distance de travail ou rapide axe Y total
				}
				
				if(($new_z != $last_z)){ //si nouveau Z
					if($nc_absolute){$tmp_travel_z = NNS_numberdiff($last_z,$new_z);}else{$tmp_travel_z = abs($new_z);} //calcule distance Z en absolue ou relatif
					if($tmptype=='fast'){$new_speed = $speed_fast_Z;} //mise a jour de la distance de travail ou rapide axe Z total
				}
				
				if(($new_x != $last_x) && ($new_y == $last_y)){ //deplacement seulement X
					$tmp_travel_tool = $tmp_travel_x; //deplacement d'outil sur l'axe X
				}elseif(($new_x == $last_x) && ($new_y != $last_y)){ //deplacement seulement Y
					$tmp_travel_tool = $tmp_travel_y; //deplacement d'outil sur l'axe Y
				}elseif(($new_x != $last_x) && ($new_y != $last_y)){ //deplacement X et Y
					$tmp_travel_tool = sqrt($tmp_travel_x*$tmp_travel_x+$tmp_travel_y*$tmp_travel_y); //deplacement d'outil sur l'axe X et Y : diagonale
				}
				
				if(isset($tmp_travel_tool) && ($new_z != $last_z)){ //deplacement Z en plus
					$tmp_travel_tool = sqrt($tmp_travel_tool*$tmp_travel_tool+$tmp_travel_z*$tmp_travel_z); //calcul de la distance de travail via pythagore
				}elseif(!isset($tmp_travel_tool) && ($new_z != $last_z)){ //deplacement seulement Z
					$tmp_travel_tool = $tmp_travel_z; //deplacement d'outil sur l'axe Z
				}
				
				$toolused[$tooldia.'v'.$toolangle]++;
				if(($tmp_op=='g0'||$tmp_op=='g1')&&$include_preview){$preview_array[] = array('type'=>$tmp_op, 'tool'=>$tooldia, 'toolangle'=>$toolangle, 'x'=>$new_x, 'y'=>$new_y, 'z'=>$new_z);} //donnee pour preview gd
				
				if(isset($tmp_travel_tool)){ //si la distance calculé
					if($tmptype=='work'){ //si en avance travail
						$travel_real += $tmp_travel_tool; //mise a jour de la distance de travail total
						$time_work += ($tmp_travel_tool/$new_speed); //mise a jour du temps de travail total
						$comment_time[$lines_comment]['time_work'] += ($tmp_travel_tool/$new_speed); //mise a jour du temps de travail total du commentaire
					}elseif($tmptype=='fast'){ //si en avance rapide
						$travel_fast_real += $tmp_travel_tool; //mise a jour de la distance de rapide total
						$time_fast += ($tmp_travel_tool/$new_speed); //mise a jour du temps de rapide total
						$comment_time[$lines_comment]['time_fast'] += ($tmp_travel_tool/$new_speed); //mise a jour du temps de rapide total du commentaire
					}
				}
				
				unset($tmp_travel_tool); //-----------------------------------free de la distance calculé
				$last_x = $new_x; $last_y = $new_y; $last_z = $new_z; //définition du nouveau XYZ pour prochaine boucle
				$last_speed = $new_speed; //définition du nouveau F pour prochaine boucle
			}else{
				$lines_ignored++; //si ligne ignorée
			}
		}
	}
	//print_r($toolused);die;
	for($i=0;$i<count($tools)+1;$i++){if(is_array($tools[$i])){if(isset($tools[$i])){if(isset($nctools[$tools[$i]['d']])){$tmptools['t'.$nctools[$tools[$i]['d']]]=$tools[$i];}elseif(isset($toolused[$tools[$i]['d'].'v'.$tools[$i]['v']])){if($toolused[$tools[$i]['d'].'v'.$tools[$i]['v']]>2){$tmptools[$tools[$i]['d'].'v'.$tools[$i]['v']]=$tools[$i];}}}}} //Fusion de $tools et $nctools en $tmptools : array[Tn]=array(dia,vangle)
	if(!isset($tmptools)){$tmptools=false;} //si pas d'outil detecté
	
	for($i=0;$i<count($comment_time)+1;$i++){if(isset($comment_time[$i])){if(($comment_time[$i]['time_fast'] >= 0.016) || ($comment_time[$i]['time_work'] >= 0.016)){$tmpcommenttime[] = array('unit'=>'sec','comment'=>$comment_time[$i]['comment'],'absolute'=>$comment_time[$i]['absolute'],'total'=>ceil(($comment_time[$i]['time_fast']+$comment_time[$i]['time_work'])*60),'work'=>ceil($comment_time[$i]['time_work']*60),'fast'=>ceil($comment_time[$i]['time_fast']*60));}}} //recuperation des commentaires avec un temps >= 1sec : $tmpcommenttime
	if(!isset($tmpcommenttime)){$tmpcommenttime=false;} //si pas de commentaires avec un temps >= 1sec
	
	for($i=0;$i<count($preview_array);$i++){if(isset($nctools[$preview_array[$i]['tool']])){$preview_array[$i]['nctool'] = $nctools[$preview_array[$i]['tool']];}else{$preview_array[$i]['nctool'] = 0;}}
	if(!isset($preview_array)){$preview_array=false;} //si pas de données pour aperçu
	
	$return_array['preview_data'] = $preview_array; //donnée pour aperçu
	$return_array['info']['nc_tools'] = $tmptools; //tableau d'outils
	$return_array['info']['nc_operations'] = $tmpcommenttime; //tableau de duree entre commentaire
	$return_array['info']['nc_times'] = array('unit'=>'sec','total'=>ceil(($time_work+$time_fast)*60),'work'=>ceil($time_work*60),'fast'=>ceil($time_fast*60));
	$return_array['info']['nc_travels'] = array('unit'=>'mm','total'=>round($travel_real+$travel_fast_real+$travel_radial+$travel_drill,2,PHP_ROUND_HALF_UP),'work'=>round($travel_real,2,PHP_ROUND_HALF_UP),'fast'=>round($travel_fast_real,2,PHP_ROUND_HALF_UP),'radius'=>round($travel_radial,2,PHP_ROUND_HALF_UP),'drill'=>round($travel_drill,2,PHP_ROUND_HALF_UP));
	$return_array['info']['nc_limits'] = array('unit'=>'mm','x_min'=>round($minX,2,PHP_ROUND_HALF_DOWN),'y_min'=>round($minY,2,PHP_ROUND_HALF_DOWN),'z_min'=>round($minZ,2,PHP_ROUND_HALF_DOWN),'z_min_work'=>round($minZ_work,2,PHP_ROUND_HALF_DOWN),'x_max'=>round($maxX,2,PHP_ROUND_HALF_UP),'y_max'=>round($maxY,2,PHP_ROUND_HALF_UP),'z_max'=>round($maxZ,2,PHP_ROUND_HALF_UP),'z_max_work'=>round($maxZ_work,2,PHP_ROUND_HALF_UP),'x'=>round($maxX-$minX,2,PHP_ROUND_HALF_UP),'y'=>round($maxY-$minY,2,PHP_ROUND_HALF_UP),'z'=>round($maxZ-$minZ,2,PHP_ROUND_HALF_UP));
	$return_array['info']['nc_lines'] = array('nc'=>$lines_count,'comment'=>$lines_comment,'ignored'=>$lines_ignored,'g0'=>$lines_g0,'g1'=>$lines_g1,'g2'=>$lines_g2,'g3'=>$lines_g3,'g81-g83'=>$lines_drill);
	
	if($speed_percent<=1){$speed_percent=$speed_percent*100;} //ratio vers pourcent
	if($speed_percent<100){if(isset($return_array['info']['nc_times'])){$return_array['info']['nc_percent']['unit'] = '%'; $return_array['info']['nc_percent']['speed'] = $speed_percent;$return_array['info']['nc_times_percent'] = array('unit'=>'sec','work'=>ceil($return_array['info']['nc_times']['work']/($speed_percent/100)),'fast'=>ceil($return_array['info']['nc_times']['fast']/($speed_percent/100)),'total'=>ceil($return_array['info']['nc_times']['total']/($speed_percent/100)));}} //si un pourcentage d'avance < 100% defini
	
	$return_array['info']['memorypeak']['extractdata'] = memory_get_peak_usage(true)/1024;
	$return_array['info']['compute_time']['extractdata'] = microtime(true)-$computetime;
	return true;
}



function NNS_NC_GeneratePreview(&$return_array,$returntoarray=false,$forceaa=false,$minimal_preview=false){ //XXX: NNS_NC_GeneratePreview()
	if($return_array['fatal_error']){return false;}
	$computetime = microtime(true);
	
	$preview_array = $return_array['preview_data']; $gdsettings = $return_array['settings']['gd'];
	$minX=$return_array['info']['nc_limits']['x_min']; $maxX=$return_array['info']['nc_limits']['x_max']; $minY=$return_array['info']['nc_limits']['y_min']; $maxY=$return_array['info']['nc_limits']['y_max']; $minZ=$return_array['info']['nc_limits']['z_min']; $maxZ=$return_array['info']['nc_limits']['z_max'];
	$minZ_work=$return_array['info']['nc_limits']['z_min_work']; $maxZ_work=$return_array['info']['nc_limits']['z_max_work']; //limite inf et sup Z en mode travail
	$preview_str_fsize = $gdsettings['str_fontsize']; $preview_str_fwidth = $gdsettings['str_fontwidth']; $preview_str_fheight = $gdsettings['str_fontheight'];
	$preview_width = $gdsettings['width']; $preview_margin = $gdsettings['margin']; $preview_grid = $gdsettings['grid']; $preview_subgrid = $gdsettings['subgrid']; $preview_arc_resolution = $gdsettings['arc_resolution'];
	$preview_str_grid = $gdsettings['str_grid']; $preview_str_axis = $gdsettings['str_axis']; $preview_str_tool = $gdsettings['str_tool']; $preview_str_fast = $gdsettings['str_fast']; $preview_str_work = $gdsettings['str_work']; $preview_str_radius = $gdsettings['str_radius'];
	
	if(function_exists('imageantialias') && $forceaa){$preview_antialiasing=true;}else{$preview_antialiasing=false;}
	
	if(0<$minX){$preview_tmp_minX=0;}else{$preview_tmp_minX=$minX;}
	if(0>$maxX){$preview_tmp_maxX=0;}else{$preview_tmp_maxX=$maxX;}
	if(0<$minY){$preview_tmp_minY=0;}else{$preview_tmp_minY=$minY;}
	if(0>$maxY){$preview_tmp_maxY=0;}else{$preview_tmp_maxY=$maxY;}
	
	if(abs($preview_tmp_maxX-$preview_tmp_minX)>abs($preview_tmp_maxY-$preview_tmp_minY)){ //calcul de la hauteur si largeur > hauteur
		$preview_height = $preview_width/((abs($preview_tmp_maxX-$preview_tmp_minX)+$preview_margin*2)/(abs($preview_tmp_maxY-$preview_tmp_minY)+$preview_margin*2)); //px
		$preview_scale = $preview_width/(abs($preview_tmp_maxX-$preview_tmp_minX)+$preview_margin*2); //px
		//$preview_offsetx = abs($preview_tmp_minX)+$preview_margin; //mm
	}else{ //correction de la hauteur si hauteur > largeur
		//$preview_height = $preview_width/((abs($preview_tmp_maxX-$preview_tmp_minX)+$preview_margin*2)/(abs($preview_tmp_maxY-$preview_tmp_minY)+$preview_margin*2)); //px
		
		$preview_height = $preview_width; //px
		
		
		$preview_scale = ($preview_width)/(abs($preview_tmp_maxY-$preview_tmp_minY)+$preview_margin*2); //px
		//$preview_offsetx = abs($preview_tmp_maxY-$preview_tmp_minY)-abs($preview_tmp_maxX-$preview_tmp_minX)-$preview_margin/2/*/2*/; //mm
		//$preview_tmp_maxX = $preview_tmp_maxY;
		
		
	//	$preview_height = $preview_width; //px
	//	$preview_tmp_maxX = $preview_tmp_maxY;
	}
	
	//$preview_scale = ($preview_width)/(abs($preview_tmp_maxX-$preview_tmp_minX)+$preview_margin*2); //px
	$return_array['info']['preview_ppm'] = round($preview_scale*1000);
	
	$preview_offsetx = abs($preview_tmp_minX)+$preview_margin; //mm
	$preview_offsety = abs($preview_tmp_minY)+$preview_margin; //mm
	$preview_fullwidth = $preview_width+($preview_str_fwidth+10)*2; //px
	
	$preview_textlines = 6;
	if(is_array($return_array['info']['nc_tools'])){$preview_textlines++;}//else{$preview_textlines--;}
	
	if(!$minimal_preview){$preview_image = imagecreatetruecolor($preview_fullwidth,$preview_height+$preview_str_fheight*$preview_textlines+4);
	}else{$preview_image = imagecreatetruecolor($preview_width,$preview_height);}
	if($preview_antialiasing){imageantialias($preview_image,$preview_antialiasing);}
	
	if(is_array($return_array['info']['nc_tools'])){$preview_textlines--;}
	
	//couleur gd : ok+opt
	$preview_background_color = imagecolorallocate($preview_image,$gdsettings['color_background'][0],$gdsettings['color_background'][1],$gdsettings['color_background'][2]); //couleur du fond
	$preview_axis_color = imagecolorallocate($preview_image,$gdsettings['color_axis'][0],$gdsettings['color_axis'][1],$gdsettings['color_axis'][2]); //couleur des axes
	$preview_grid_color = imagecolorallocate($preview_image,$gdsettings['color_grid'][0],$gdsettings['color_grid'][1],$gdsettings['color_grid'][2]); //couleur des graduations
	$preview_subgrid_color = imagecolorallocate($preview_image,$gdsettings['color_subgrid'][0],$gdsettings['color_subgrid'][1],$gdsettings['color_subgrid'][2]); //couleur des graduations
	$preview_work_color = imagecolorallocate($preview_image,$gdsettings['color_linear'][0],$gdsettings['color_linear'][1],$gdsettings['color_linear'][2]); //couleur des trajectoires en mode travail
	$preview_radius_color = imagecolorallocate($preview_image,$gdsettings['color_circular'][0],$gdsettings['color_circular'][1],$gdsettings['color_circular'][2]); //couleur des trajectoires en mode circulaire
	$preview_fast_color = imagecolorallocate($preview_image,$gdsettings['color_fast'][0],$gdsettings['color_fast'][1],$gdsettings['color_fast'][2]); //couleur des trajectoires en mode rapide
	$preview_tool_color = imagecolorallocate($preview_image,$gdsettings['color_toolpath_higher'][0],$gdsettings['color_toolpath_higher'][1],$gdsettings['color_toolpath_higher'][2]); //couleur des outils
	$preview_limits_color = imagecolorallocate($preview_image,$gdsettings['color_limits'][0],$gdsettings['color_limits'][1],$gdsettings['color_limits'][2]); //couleur des limites
	$preview_limitslines_color = imagecolorallocate($preview_image,$gdsettings['color_limitslines'][0],$gdsettings['color_limitslines'][1],$gdsettings['color_limitslines'][2]); //couleur des lignes des limites
	
	//reset background image : ok+opt
	imagefill($preview_image,0,0,$preview_background_color);
	
	//divers : ok+opt
	$preview_nbline = count($preview_array); //loop fastup
	$preview_origx = $preview_offsetx*$preview_scale; //origne x px
	$preview_origy = $preview_height-$preview_offsety*$preview_scale; //origne y px
	$preview_array[-1]=array('x'=>0,'y'=>0,'tool'=>0,'toolangle'=>0); //point de depart initial
	
	//dessin sub grille : ok+opt
	$preview_subgrid_scaled = $preview_subgrid*$preview_scale;
	if($preview_subgrid_scaled>$gdsettings['subgrid_minpx']){
		for($preview_x1=$preview_origx;$preview_x1<$preview_width;$preview_x1+=$preview_subgrid_scaled){imageline($preview_image,$preview_x1,0,$preview_x1,$preview_height,$preview_subgrid_color);} //boucle dessin grille y+
		for($preview_x1=$preview_origx;$preview_x1>0;$preview_x1-=$preview_subgrid_scaled){imageline($preview_image,$preview_x1,0,$preview_x1,$preview_height,$preview_subgrid_color);} //boucle dessin grille y-
		for($preview_y1=$preview_origy;$preview_y1<$preview_height;$preview_y1+=$preview_subgrid_scaled){imageline($preview_image,0,$preview_y1,$preview_width,$preview_y1,$preview_subgrid_color);} //boucle dessin grille x+
		for($preview_y1=$preview_origy;$preview_y1>0;$preview_y1-=$preview_subgrid_scaled){imageline($preview_image,0,$preview_y1,$preview_width,$preview_y1,$preview_subgrid_color);} //boucle dessin grille x-
	}
	
	//dessin grille : ok+opt
	$preview_grid_scaled = $preview_grid*$preview_scale;
	for($preview_x1=$preview_origx;$preview_x1<$preview_width;$preview_x1+=$preview_grid_scaled){imageline($preview_image,$preview_x1,0,$preview_x1,$preview_height,$preview_grid_color);} //boucle dessin grille y+
	for($preview_x1=$preview_origx;$preview_x1>0;$preview_x1-=$preview_grid_scaled){imageline($preview_image,$preview_x1,0,$preview_x1,$preview_height,$preview_grid_color);} //boucle dessin grille y-
	for($preview_y1=$preview_origy;$preview_y1<$preview_height;$preview_y1+=$preview_grid_scaled){imageline($preview_image,0,$preview_y1,$preview_width,$preview_y1,$preview_grid_color);} //boucle dessin grille x+
	for($preview_y1=$preview_origy;$preview_y1>0;$preview_y1-=$preview_grid_scaled){imageline($preview_image,0,$preview_y1,$preview_width,$preview_y1,$preview_grid_color);} //boucle dessin grille x-
	
	//rectangle pointille des limites : ok+opt
	if(!$minimal_preview){
		if($preview_antialiasing){imageantialias($preview_image,false);}
		$minX_px=($minX+$preview_offsetx)*$preview_scale;
		$maxX_px=($maxX+$preview_offsetx)*$preview_scale;
		$minY_px=$preview_height-($minY+$preview_offsety)*$preview_scale;
		$maxY_px=$preview_height-($maxY+$preview_offsety)*$preview_scale;
		if(!$gdsettings['ignore_limits'] && !$minimal_preview){
			NNS_imagelinedashed($preview_image,$minX_px,0,$minX_px,$preview_height,$preview_limitslines_color);
			NNS_imagelinedashed($preview_image,$maxX_px,0,$maxX_px,$preview_height,$preview_limitslines_color);
			NNS_imagelinedashed($preview_image,0,$minY_px,$preview_width,$minY_px,$preview_limitslines_color);
			NNS_imagelinedashed($preview_image,0,$maxY_px,$preview_width,$maxY_px,$preview_limitslines_color);
		}
		if($preview_antialiasing){imageantialias($preview_image,$preview_antialiasing);}
	}
	
	//dessin tracees d'outils : ok+opt
	
	if(is_array($return_array['info']['nc_tools']) && !$gdsettings['ignore_tools']){
		for($li=0;$li<$preview_nbline;$li++){ //boucle de dessin outil
			$preview_type = $preview_array[$li]['type']; //type de ligne
			if($preview_array[$li]['tool']!=0){
				if($preview_type!='g0'){
					if($maxZ_work-$minZ_work!=0){
						$preview_tool_color=NNS_imagecolorallocatefade($preview_image,$gdsettings['color_toolpath_lower'],$gdsettings['color_toolpath_higher'],($preview_array[$li]['z']-$minZ_work)/($maxZ_work-$minZ_work)); 
					}else{$preview_tool_color=imagecolorallocate($preview_image,$gdsettings['color_toolpath_lower'][0],$gdsettings['color_toolpath_lower'][1],$gdsettings['color_toolpath_lower'][2]);}
					if($preview_array[$li]['toolangle']<=0){
						if($preview_type=='g2' || $preview_type=='g3'){
							NNS_imagearcthick($preview_image,($preview_array[$li]['i']+$preview_offsetx)*$preview_scale,$preview_height-($preview_array[$li]['j']+$preview_offsety)*$preview_scale,$preview_array[$li]['r']*$preview_scale*2,$preview_array[$li]['r']*$preview_scale*2,$preview_array[$li]['startangle'],$preview_array[$li]['endangle'],($preview_array[$li]['tool']*$preview_scale),$preview_tool_color,$preview_arc_resolution);
						}elseif($preview_array[$li]['type']=='g81' || $preview_array[$li]['type']=='g83'){
							//imagefilledellipse($preview_image,($preview_array[$li-1]['x']+$preview_offsetx)*$preview_scale,$preview_height-($preview_array[$li-1]['y']+$preview_offsety)*$preview_scale,$preview_array[$li]['tool']*$preview_scale,$preview_array[$li]['tool']*$preview_scale,$preview_tool_color);
							imagefilledellipse($preview_image,($preview_array[$li]['x']+$preview_offsetx)*$preview_scale,$preview_height-($preview_array[$li]['y']+$preview_offsety)*$preview_scale,$preview_array[$li]['tool']*$preview_scale,$preview_array[$li]['tool']*$preview_scale,$preview_tool_color);
						}else{
							NNS_imagelinethick($preview_image,($preview_array[$li-1]['x']+$preview_offsetx)*$preview_scale,$preview_height-($preview_array[$li-1]['y']+$preview_offsety)*$preview_scale,($preview_array[$li]['x']+$preview_offsetx)*$preview_scale,$preview_height-($preview_array[$li]['y']+$preview_offsety)*$preview_scale,($preview_array[$li]['tool']*$preview_scale),$preview_tool_color);
						}
					}else{
						if($preview_array[$li]['toolangle']>0){
							$dZ = (tan(deg2rad($preview_array[$li]['toolangle']))*abs($preview_array[$li]['z']))*$preview_scale*2;
							if($dZ>$preview_array[$li]['tool']*$preview_scale){$dZ=$preview_array[$li]['tool']*$preview_scale;}
							if($preview_type=='g2' || $preview_type=='g3'){NNS_imagearcthickV($preview_image,($preview_array[$li]['i']+$preview_offsetx)*$preview_scale,$preview_height-($preview_array[$li]['j']+$preview_offsety)*$preview_scale,$preview_array[$li]['r']*$preview_scale*2,$preview_array[$li]['r']*$preview_scale*2,$preview_array[$li]['startangle'],$preview_array[$li]['endangle'],$lastdZ,$dZ,$preview_tool_color,$preview_arc_resolution);
							}else{NNS_imagelinethickV($preview_image,($preview_array[$li-1]['x']+$preview_offsetx)*$preview_scale,$preview_height-($preview_array[$li-1]['y']+$preview_offsety)*$preview_scale,($preview_array[$li]['x']+$preview_offsetx)*$preview_scale,$preview_height-($preview_array[$li]['y']+$preview_offsety)*$preview_scale,$lastdZ,$dZ,$preview_tool_color);}
							$lastdZ = $dZ;
						}
					}
				}
			}
		}
		
		$preview_tool_color = imagecolorallocate($preview_image,$gdsettings['color_toolpath_higher'][0],$gdsettings['color_toolpath_higher'][1],$gdsettings['color_toolpath_higher'][2]); //couleur des outils
		if(!$minimal_preview){
			$preview_str_toolslist = "Outil(s) : ";
			while($tmp=current($return_array['info']['nc_tools'])){
				if($tmp['v']==0){
					$preview_str_toolslist.=' ['.chr(1).' '.$tmp['d'].$tmp['d_unit'].'] ';
				}else{
					$preview_str_toolslist.=' ['.chr(1).' '.$tmp['d'].$tmp['d_unit'].' '.($tmp['v']*2).$tmp['v_unit'].'] ';
				}
				next($return_array['info']['nc_tools']);
			}
			
			imageline($preview_image,0,$preview_height+$preview_str_fheight*$preview_textlines+3,$preview_fullwidth,$preview_height+$preview_str_fheight*$preview_textlines+3,$preview_subgrid_color);
			$preview_toolslist_width = strlen($preview_str_toolslist)*$preview_str_fwidth;
			$preview_toolslist_xpos = ($preview_fullwidth-($preview_toolslist_width))/2;
			imagestring($preview_image,$preview_str_fsize,$preview_toolslist_xpos,$preview_height+$preview_str_fheight*$preview_textlines+3,$preview_str_toolslist,$preview_tool_color);
		}
	}
	
	//dessin origine + trait d'axe : ok+opt
	imageline($preview_image,0,$preview_origy,$preview_width-$gdsettings['axis_fontwidth']-2,$preview_origy,$preview_axis_color); //dessin axe x
	imageline($preview_image,$preview_origx,$gdsettings['axis_fontheight']+2,$preview_origx,$preview_height,$preview_axis_color); //dessin axe y
	NNS_imagearrow($preview_image,$preview_width-$gdsettings['axis_fontwidth']-2,$preview_origy,0,10,8,'x+',$preview_axis_color,true); //dessin fleche x
	imagestring($preview_image,$gdsettings['axis_fontsize'],$preview_width-$gdsettings['axis_fontwidth'],$preview_origy-$gdsettings['axis_fontheight']/2,'X',$preview_axis_color); //dessin text
	NNS_imagearrow($preview_image,$preview_origx,$gdsettings['axis_fontheight']+2,0,10,8,'y-',$preview_axis_color,true); //dessin fleche y
	imagestring($preview_image,$gdsettings['axis_fontsize'],$preview_origx-$gdsettings['axis_fontwidth']/2+1,0,'Y',$preview_axis_color); //dessin text
	imageellipse($preview_image,$preview_origx,$preview_origy,15,15,$preview_axis_color); imagefilledarc($preview_image,$preview_origx,$preview_origy,15,15,-90,0,$preview_axis_color,IMG_ARC_PIE); imagefilledarc($preview_image,$preview_origx,$preview_origy,15,15,90,180,$preview_axis_color,IMG_ARC_PIE); //dessin origine
	
	//boucle de dessin travail : ok
	if(!$gdsettings['ignore_work']){
		for($li=0;$li<$preview_nbline;$li++){
			if($preview_array[$li]['type']!='g0'){
				if($preview_array[$li]['type']=='g2' || $preview_array[$li]['type']=='g3'){
					NNS_imagearc($preview_image,($preview_array[$li]['i']+$preview_offsetx)*$preview_scale,$preview_height-($preview_array[$li]['j']+$preview_offsety)*$preview_scale,$preview_array[$li]['r']*$preview_scale*2,$preview_array[$li]['r']*$preview_scale*2,$preview_array[$li]['startangle'],$preview_array[$li]['endangle'],$preview_radius_color,$preview_arc_resolution);
				}elseif($preview_array[$li]['type']=='g81' || $preview_array[$li]['type']=='g83'){
					imageline($preview_image,($preview_array[$li-1]['x']+$preview_offsetx)*$preview_scale,$preview_height-($preview_array[$li-1]['y']+$preview_offsety)*$preview_scale,($preview_array[$li]['x']+$preview_offsetx)*$preview_scale,$preview_height-($preview_array[$li]['y']+$preview_offsety)*$preview_scale,$preview_fast_color);
				}else{
					imageline($preview_image,($preview_array[$li-1]['x']+$preview_offsetx)*$preview_scale,$preview_height-($preview_array[$li-1]['y']+$preview_offsety)*$preview_scale,($preview_array[$li]['x']+$preview_offsetx)*$preview_scale,$preview_height-($preview_array[$li]['y']+$preview_offsety)*$preview_scale,$preview_work_color);
				}
			}
		}
	}
	
	//boucle de dessin rapide : ok
	if(!$gdsettings['ignore_fast']){
		for($li=0;$li<$preview_nbline;$li++){
			if($preview_array[$li]['type']=='g0'){
				imageline($preview_image,($preview_array[$li-1]['x']+$preview_offsetx)*$preview_scale,$preview_height-($preview_array[$li-1]['y']+$preview_offsety)*$preview_scale,($preview_array[$li]['x']+$preview_offsetx)*$preview_scale,$preview_height-($preview_array[$li]['y']+$preview_offsety)*$preview_scale,$preview_fast_color);
			}
		}
	}
	
	if(!$minimal_preview){
		//info : ok
		imageline($preview_image,0,$preview_height+$preview_str_fheight*($preview_textlines-2)+3,$preview_fullwidth,$preview_height+$preview_str_fheight*($preview_textlines-2)+3,$preview_subgrid_color);
		$preview_str_grid = "[$preview_str_grid] "; $preview_str_axis = "[$preview_str_axis] "; $preview_str_tool = "[$preview_str_tool] "; $preview_str_fast = "[$preview_str_fast] "; $preview_str_work = "[$preview_str_work] "; $preview_str_radius = "[$preview_str_radius]";
		$preview_str_lenght = strlen($preview_str_grid.$preview_str_axis.$preview_str_tool.$preview_str_fast.$preview_str_work.$preview_str_radius);
		$preview_str_width = $preview_str_lenght*$preview_str_fwidth;
		$preview_str_xpos = ($preview_fullwidth-$preview_str_width)/2;
		imagestring($preview_image,$preview_str_fsize,$preview_str_xpos,$preview_height+$preview_str_fheight*($preview_textlines-2)+3,str_pad($preview_str_grid,$preview_str_lenght),$preview_grid_color);
		imagestring($preview_image,$preview_str_fsize,$preview_str_xpos,$preview_height+$preview_str_fheight*($preview_textlines-2)+3,str_pad(str_pad($preview_str_axis,strlen($preview_str_grid.$preview_str_axis),' ', STR_PAD_LEFT),$preview_str_lenght),$preview_axis_color);
		imagestring($preview_image,$preview_str_fsize,$preview_str_xpos,$preview_height+$preview_str_fheight*($preview_textlines-2)+3,str_pad(str_pad($preview_str_tool,strlen($preview_str_grid.$preview_str_axis.$preview_str_tool),' ', STR_PAD_LEFT),$preview_str_lenght),$preview_tool_color);
		imagestring($preview_image,$preview_str_fsize,$preview_str_xpos,$preview_height+$preview_str_fheight*($preview_textlines-2)+3,str_pad(str_pad($preview_str_fast,strlen($preview_str_grid.$preview_str_axis.$preview_str_tool.$preview_str_fast),' ', STR_PAD_LEFT),$preview_str_lenght),$preview_fast_color);
		imagestring($preview_image,$preview_str_fsize,$preview_str_xpos,$preview_height+$preview_str_fheight*($preview_textlines-2)+3,str_pad(str_pad($preview_str_work,strlen($preview_str_grid.$preview_str_axis.$preview_str_tool.$preview_str_fast.$preview_str_work),' ', STR_PAD_LEFT),$preview_str_lenght),$preview_work_color);
		imagestring($preview_image,$preview_str_fsize,$preview_str_xpos,$preview_height+$preview_str_fheight*($preview_textlines-2)+3,str_pad($preview_str_radius,$preview_str_lenght,' ', STR_PAD_LEFT),$preview_radius_color);
			
		//temps : ok
		$time_total_str = NNS_sec2str($return_array['info']['nc_times']['total']);
		$preview_time = $gdsettings['str_time'] . " : " . $time_total_str;
			
		if(isset($return_array['info']['nc_times_percent'])){ //si un pourcentage d'avance < 100% defini
			$time_percent_total_str = NNS_sec2str($return_array['info']['nc_times_percent']['total']);
			$preview_time .= "  -  " . $gdsettings['str_timepercent'] . " (Avance {$return_array['settings']['cnc']['speed_percent']}%) : " . $time_percent_total_str;
		}
		
		imageline($preview_image,0,$preview_height+$preview_str_fheight*($preview_textlines-1)+3,$preview_fullwidth,$preview_height+$preview_str_fheight*($preview_textlines-1)+3,$preview_subgrid_color);
		$preview_time_width = strlen($preview_time)*$preview_str_fwidth;
		$preview_time_xpos = ($preview_width-($preview_time_width))/2;
		imagestring($preview_image,$preview_str_fsize,$preview_time_xpos,$preview_height+$preview_str_fheight*($preview_textlines-1)+3,$preview_time,$preview_limits_color);
		
		//limites
		$preview_str_minX = ' '.round($minX,2)."mm ";
		$preview_str_maxX = ' '.round($maxX,2)."mm ";
		$preview_str_fullX = round($maxX-$minX,2)."mm";
		$preview_str_minY = ' '.round($minY,2)."mm ";
		$preview_str_maxY = ' '.round($maxY,2)."mm ";
		$preview_str_fullY = round($maxY-$minY,2)."mm";
		$preview_str_Z = "Z mini : ".round($minZ,2)."mm  <>  Z maxi : ".round($maxZ,2)."mm";
		$preview_str_width = strlen($preview_str_Z)*$preview_str_fwidth;
		
		//X min
		NNS_imagearrow($preview_image,$minX_px,$preview_height+$preview_str_fheight-1,$preview_str_fheight,6,6,'y-',$preview_limits_color,true);
		imagestring($preview_image,$preview_str_fsize,$minX_px+2,$preview_height,$preview_str_minX,$preview_limits_color);
		imageline($preview_image,$minX_px+1,$preview_height+$preview_str_fheight,$minX_px+strlen($preview_str_minX)*$preview_str_fwidth,$preview_height+$preview_str_fheight,$preview_limits_color);
		
		//X max
		NNS_imagearrow($preview_image,$maxX_px,$preview_height+$preview_str_fheight-1,$preview_str_fheight,6,6,'y-',$preview_limits_color,true);
		imagestring($preview_image,$preview_str_fsize,$maxX_px-strlen($preview_str_maxX)*$preview_str_fwidth,$preview_height,$preview_str_maxX,$preview_limits_color);
		imageline($preview_image,$maxX_px-1,$preview_height+$preview_str_fheight,$maxX_px-strlen($preview_str_maxX)*$preview_str_fwidth,$preview_height+$preview_str_fheight,$preview_limits_color);
		
		//X total
		if($maxX-$minX!=0){
			NNS_imagearrow($preview_image,$minX_px,$preview_height+$preview_str_fheight*2,$preview_str_fheight,6,6,'y-',$preview_limits_color,true);
			NNS_imagearrow($preview_image,$maxX_px,$preview_height+$preview_str_fheight*2,$preview_str_fheight,6,6,'y-',$preview_limits_color,true);
			imageline($preview_image,$minX_px+1,$preview_height+$preview_str_fheight*2,$maxX_px-1,$preview_height+$preview_str_fheight*2,$preview_limits_color);
			imagestring($preview_image,$preview_str_fsize,($minX_px+($maxX_px-$minX_px)/2)-strlen($preview_str_fullX)*$preview_str_fwidth/2,$preview_height+$preview_str_fheight,$preview_str_fullX,$preview_limits_color);
		}
	
		//Y min
		NNS_imagearrow($preview_image,$preview_width+$preview_str_fheight-1,$minY_px,$preview_str_fheight,6,6,'x-',$preview_limits_color,true);
		imagestringup($preview_image,$preview_str_fsize,$preview_width,$minY_px-2,$preview_str_minY,$preview_limits_color);
		imageline($preview_image,$preview_width+$preview_str_fheight,$minY_px-1,$preview_width+$preview_str_fheight,$minY_px-strlen($preview_str_minY)*$preview_str_fwidth,$preview_limits_color);
		
		//Y max
		NNS_imagearrow($preview_image,$preview_width+$preview_str_fheight-1,$maxY_px,$preview_str_fheight,6,6,'x-',$preview_limits_color,true);
		imagestringup($preview_image,$preview_str_fsize,$preview_width,$maxY_px+strlen($preview_str_maxY)*$preview_str_fwidth,$preview_str_maxY,$preview_limits_color);
		imageline($preview_image,$preview_width+$preview_str_fheight,$maxY_px+1,$preview_width+$preview_str_fheight,$maxY_px+strlen($preview_str_maxY)*$preview_str_fwidth,$preview_limits_color);
		
		//Y total
		if($maxY-$minY!=0){
			NNS_imagearrow($preview_image,$preview_width+$preview_str_fheight*2,$minY_px,$preview_str_fheight,6,6,'x-',$preview_limits_color,true);
			NNS_imagearrow($preview_image,$preview_width+$preview_str_fheight*2,$maxY_px,$preview_str_fheight,6,6,'x-',$preview_limits_color,true);
			imageline($preview_image,$preview_width+$preview_str_fheight*2,$minY_px-1,$preview_width+$preview_str_fheight*2,$maxY_px+1,$preview_limits_color);
			imagestringup($preview_image,$preview_str_fsize,$preview_width+$preview_str_fheight,($minY_px+($maxY_px-$minY_px)/2)+strlen($preview_str_fullY)*$preview_str_fwidth/2,$preview_str_fullY,$preview_limits_color);
		}
		
		//Z
		imagestring($preview_image,$preview_str_fsize,1+($preview_fullwidth-$preview_str_width)/2,$preview_height+$preview_str_fheight*($preview_textlines-3),$preview_str_Z,$preview_limits_color);
		imageline($preview_image,($preview_fullwidth-$preview_str_width)/2,$preview_height+$preview_str_fheight*($preview_textlines-2),($preview_fullwidth+$preview_str_width)/2,$preview_height+$preview_str_fheight*($preview_textlines-2),$preview_limits_color);
	}
	
	if($returntoarray===true){ //retourne l'image gd dans le tableau $return_array
		$return_array['gd_image']=$preview_image;
		$return=true;
	}elseif($returntoarray!==false){ //sauvegarde l'image dans un fichier
		$return_array['info']['preview_file']=$returntoarray.'.png';
		$return=imagepng($preview_image,$returntoarray.'.png');
	}else{ //retourne directement l'image
		header('Content-Type: image/png');
		$return=imagepng($preview_image);
	}
	
	$return_array['info']['compute_time']['generatepreview'] = microtime(true)-$computetime;
	$return_array['info']['memorypeak']['generatepreview'] = memory_get_peak_usage(true)/1024;
	return $return;
}


function NNS_PNG_FillInfo($file,$title=false,$description=false,$software=false,$comment=false){ //XXX: NNS_PNG_FillInfo()
	$file_cnt=file_get_contents($file);
	if(substr($file_cnt,0,4)=="\x89\x50\x4E\x47"){
		$file_begin=substr($file_cnt,0,strrpos($file_cnt,"\x0\x0\x0\x0IEND"));
		$file_end=substr($file_cnt,strrpos($file_cnt,"\x0\x0\x0\x0IEND"));
		$file_toadd='';
		
		if($title!==false){
			$chunk='tEXt';
			$tag='Title';
			$text=$title;
			$length=pack('N',strlen($tag.$text)+1);
			$crc=pack('N',crc32("$tag\x0$text"));
			$file_toadd.="$length$chunk$tag\x0$text$crc";
		}
		
		if($description!==false){
			$chunk='tEXt';
			$tag='Description';
			$text=$description;
			$length=pack('N',strlen($tag.$text)+1);
			$crc=pack('N',crc32("$tag\x0$text"));
			$file_toadd.="$length$chunk$tag\x0$text$crc";
		}
		
		if($software!==false){
			$chunk='tEXt';
			$tag='Software';
			$text=$software;
			$length=pack('N',strlen($tag.$text)+1);
			$crc=pack('N',crc32("$tag\x0$text"));
			$file_toadd.="$length$chunk$tag\x0$text$crc";
		}
		
		if($comment!==false){
			$chunk='tEXt';
			$tag='Comment';
			$text=$comment;
			$length=pack('N',strlen($tag.$text)+1);
			$crc=pack('N',crc32("$tag\x0$text"));
			$file_toadd.="$length$chunk$tag\x0$text$crc";
		}
		
		file_put_contents($file,$file_begin.$file_toadd.$file_end);return true;}else{return false;}
}


function NNS_PNG_PPM($file,$ppm=false){ //XXX: NNS_PNG_FillInfo()
	$file_cnt=file_get_contents($file);
	if(substr($file_cnt,0,4)=="\x89\x50\x4E\x47"){
		$file_begin=substr($file_cnt,0,strpos($file_cnt,'IDAT')-4);
		$file_end=substr($file_cnt,strpos($file_cnt,'IDAT')-4);
		
		if($ppm!==false){
			$ppm=round($ppm);
			$chunk='pHYs';
			$text=pack('N',$ppm).pack('N',$ppm)."\x01";
			$length=pack('N',strlen($text));
			$crc=pack('N',crc32($chunk.$text));
			$file_toadd="$length$chunk$text$crc";
		}
		
		file_put_contents($file,$file_begin.$file_toadd.$file_end);return true;}else{return false;}
}











//------------------------------------------------------------------ XXX : Start
if(!$ncscript_is_included){
		NNS_NC_Settings($ncarray); //Chargement des parametres
		$version = 'NC Duration Estimator v2-';
		$version .= date ("Ymd-Hi",filemtime(__FILE__));
		$copyright = 'NNS - '.date("Y");
		$run_from_cmd = $ncarray['settings']['general']['run_from_cmd'];
		$target_dir = $ncarray['settings']['general']['target_dir'];
		
	//------------------------------------------------------------------ XXX : Definition des couleurs si terminal
		if($ncarray['settings']['general']['run_from_cmd'] && ((strtoupper(substr(PHP_OS,0,3))==='WIN' && NNS_fileexists_pathvar("ansicon"))||(strtoupper(substr(PHP_OS,0,3))!=='WIN'))){$coloredcmd=true;}else{$coloredcmd=false;}
		if($ncarray['settings']['general']['run_from_cmd'] || $coloredcmd){ //colorisation cmd
			$cmdcolok="\033[1;32m"; //#03ff00 vert
			$cmdcolwarn="\033[1;33m"; //#ffcb00 orange-jaune
			$cmdcolerror="\033[1;31m"; //#ff0000 rouge
			$cmdcolcomm="\033[1;36m"; //#00fffb cyan
			$cmdcolreset=chr(27)."\033[0m";
		}else{$cmdcolok='';$cmdcolwarn='';$cmdcolerror='';$cmdcolcomm='';$cmdcolreset='';}
	
	//------------------------------------------------------------------ XXX : Recuperation des informations envoye
		if(isset($_POST["submit"])){ //----------------------------------------------- si formulaire envoye
			$ncarray['settings']['cnc']['speed_fast_XY'] = $_POST["fastxy"]; //rapide XY
			$ncarray['settings']['cnc']['speed_fast_Z'] = $_POST["fastz"]; //rapide Z
			$ncarray['settings']['cnc']['speed_percent'] = $_POST["percent"]; //pourcentage vitesse max
			
			if(isset($_POST["ignorepreviewfast"])){$ncarray['settings']['gd']['ignore_fast']=true; $ignorepreviewfastchecked = 'checked="checked"';}else{$ncarray['settings']['gd']['ignore_fast']=false; $ignorepreviewfastchecked = '';}
			if(isset($_POST["ignorepreviewwork"])){$ncarray['settings']['gd']['ignore_work']=true; $ignorepreviewworkchecked = 'checked="checked"';}else{$ncarray['settings']['gd']['ignore_work']=false; $ignorepreviewworkchecked = '';}
			if(isset($_POST["ignorepreviewtool"])){$ncarray['settings']['gd']['ignore_tools']=true; $ignorepreviewtoolchecked = 'checked="checked"';}else{$ncarray['settings']['gd']['ignore_tools']=false; $ignorepreviewtoolchecked = '';}
			if(isset($_POST["ignorepreviewlimits"])){$ncarray['settings']['gd']['ignore_limits']=true; $ignorepreviewlimitschecked = 'checked="checked"';}else{$ncarray['settings']['gd']['ignore_limits']=false; $ignorepreviewlimitschecked = '';}
			
			if(isset($_POST["modedebug"])){$debug=true; $debugchecked = 'checked="checked"';}else{$debug=false; $debugchecked = '';} //mode debug?
			if(basename($_FILES["ncfile"]["name"])!=''){$ncrealname_str = ' ('.basename($_FILES["ncfile"]["name"]).')';}else{$ncrealname_str = '';} //nom reel du fichier
		}elseif($run_from_cmd){
			$cmdhandle = fopen("php://stdin","r");
			echo "Vitesse d'avance rapide des axes X et Y en mm/min (Vitesse max en mode travail) : defaut = {$cmdcolwarn}{$ncarray['settings']['cnc']['speed_fast_XY_reset']}{$cmdcolreset}\n > {$cmdcolok}";
			$ncarray['settings']['cnc']['speed_fast_XY'] = str_replace(array("\r\n","\n",'mm/min'),'',fgets($cmdhandle));
			echo "{$cmdcolreset}Vitesse d'avance rapide de l'axe Z en mm/min (Vitesse max en mode travail) : defaut = {$cmdcolwarn}{$ncarray['settings']['cnc']['speed_fast_Z_reset']}{$cmdcolreset}\n > {$cmdcolok}";
			$ncarray['settings']['cnc']['speed_fast_Z'] = str_replace(array("\r\n","\n",'mm/min'),'',fgets($cmdhandle));
			echo "{$cmdcolreset}Poucentage d'avance maximum en pourcent : defaut = {$cmdcolwarn}{$ncarray['settings']['cnc']['speed_percent_reset']}{$cmdcolreset}\n > {$cmdcolok}";
			$ncarray['settings']['cnc']['speed_percent'] = str_replace(array("\r\n","\n",'%'),'',fgets($cmdhandle));
			echo "{$cmdcolreset}";
			$debug=false; $debugchecked = ''; //mode debug : non
			$ncrealname_str = ''; //nom reel du fichier
		}
	
	//------------------------------------------------------------------ XXX : Headers HTML si hors console
	if(!$run_from_cmd){
	?>
	<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
	<html xmlns="http://www.w3.org/1999/xhtml">
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-15" />
		<title><?php echo $version; ?></title>
		<style type="text/css">
			body {background-color:#706963;font-family:Arial;font-size:11px;}
			* {vertical-align:middle;text-shadow:0 0 5px rgba(0,0,0,0.5);}
			.parseok {color:#03ff00;font-weight:bold;}
			.parsewarn {color:#ffcb00;}
			.parseerror {color:#f00;font-weight:bolder;}
			.parsecomm {color:#00fffb;}
			#nccontener {border:1px dotted #FFFFFF;display:none;white-space: pre;}
			.ncfullline {border-bottom:1px dashed #B8AE6B;}
			.ncmenu {text-align:center;font-weight:bolder;color:#ffcb00;}
			#submitform {text-align: center; color: #ffcb00;}
			#submitform input {border:1px solid #E3E3E3 ; border-radius:3px;margin:2px;background-color: #9f9f9f;}
		</style>
	</head>
	<body>
	<form action="" method="post" enctype="multipart/form-data" id="submitform">
	    Fichier NC : <input type="file" name="ncfile" id="ncfile"><?php echo $ncrealname_str;?><br/>
		<table border="0" cellpadding="0" cellspacing="0" style="width:100%">
	    <tr><td style="text-align:right;">Vitesse d'avance rapide des axes X et Y : </td><td style="text-align:left;"><input size="6" maxlength="4" name="fastxy" type="text" id="fastxy" value="<?php echo $ncarray['settings']['cnc']['speed_fast_XY'];?>" /> mm/min (Vitesse max en mode travail)</td></tr>
	    <tr><td style="text-align:right;">Vitesse d'avance rapide de l'axe Z : </td><td style="text-align:left;"><input size="4" maxlength="4" name="fastz" type="text" id="fastxy" value="<?php echo $ncarray['settings']['cnc']['speed_fast_Z'];?>" /> mm/min (Vitesse max en mode travail)</td></tr>
	    <tr><td style="text-align:right;">Poucentage d'avance maximum : </td><td style="text-align:left;"><input size="3" maxlength="3" name="percent" type="text" id="fastxy" value="<?php echo $ncarray['settings']['cnc']['speed_percent'];?>" /> %</td></tr>
	    <tr><td colspan="2">&nbsp;</td></tr>
	    <tr><td style="text-align:right;">Aperçu : Ignorer les parcours rapide : </td><td style="text-align:left;"><input type="checkbox" name="ignorepreviewfast" id="ignorepreviewfast" <?php echo $ignorepreviewfastchecked;?>/></td></tr>
	    <tr><td style="text-align:right;">Aperçu : Ignorer les parcours lineaire  : </td><td style="text-align:left;"><input type="checkbox" name="ignorepreviewwork" id="ignorepreviewwork" <?php echo $ignorepreviewworkchecked;?>/></td></tr>
	    <tr><td style="text-align:right;">Aperçu : Ignorer les parcours tracer d'outil : </td><td style="text-align:left;"><input type="checkbox" name="ignorepreviewtool" id="ignorepreviewtool" <?php echo $ignorepreviewtoolchecked;?>/></td></tr>
	    <tr><td style="text-align:right;">Aperçu : Ignorer les limites de parcours : </td><td style="text-align:left;"><input type="checkbox" name="ignorepreviewlimits" id="ignorepreviewlimits" <?php echo $ignorepreviewlimitschecked;?>/></td></tr>
	    <tr><td colspan="2">&nbsp;</td></tr>
	    <tr><td style="text-align:right;">Mode Debug : </td><td style="text-align:left;"><input type="checkbox" name="modedebug" id="modedebug" <?php echo $debugchecked;?>/></td></tr>
	    </table>
	    <br/><br/>
	    <input type="submit" value="Envoyer le programme NC" name="submit" style="cursor: pointer;">
	</form>
	<br/>
	
	<?php
	}
	
	
	//------------------------------------------------------------------ XXX : Supprime les fichiers de plus de 15min
	if(!$run_from_cmd){
		NNS_FlushFolder($target_dir,$ncarray['settings']['general']['cache_duration']);
	}
	
	if(isset($_POST["submit"]) || $run_from_cmd){ //--------------------- XXX : si formulaire envoye
		if(!$run_from_cmd){ //si via le navigateur
			$ncrealname = basename($_FILES["ncfile"]["name"]); //nom reel du fichier
			$ncfile = $target_dir.$ncrealname; //chemin du fichier sur le serveur
		}else{ //si via cmd
			$ncfile = $ncarray['settings']['general']['query'];//implode(' ',$argv); //chemin du fichier sur le serveur
			$ncrealname = end(explode('/',str_replace('\\','/',$ncfile))); //nom reel du fichier
		}
		
		$ncfile_ext = strtolower(end(explode('.',$ncfile))); //extension du fichier
		
		if(!is_numeric($ncarray['settings']['cnc']['speed_fast_XY']) || $ncarray['settings']['cnc']['speed_fast_XY']<=0){$ncarray['settings']['cnc']['speed_fast_XY'] = $ncarray['settings']['cnc']['speed_fast_XY_reset'];} //verif de la vitesse rapide XY
		if(!is_numeric($ncarray['settings']['cnc']['speed_fast_Z']) || $ncarray['settings']['cnc']['speed_fast_Z']<=0){$ncarray['settings']['cnc']['speed_fast_Z'] = $ncarray['settings']['cnc']['speed_fast_Z_reset'];} //verif de la vitesse rapide Z
		if(!is_numeric($ncarray['settings']['cnc']['speed_percent']) || $ncarray['settings']['cnc']['speed_percent']<=0){$ncarray['settings']['cnc']['speed_percent'] = $ncarray['settings']['cnc']['speed_percent_reset'];} //verif du poucentage
		if(!$run_from_cmd && $ncrealname!==''){
			if(!move_uploaded_file($_FILES["ncfile"]["tmp_name"],$ncfile)){
				$ncarray['fatal_error'] = "Echec lors de l'envoi du fichier : '$ncrealname'";
			}
		} //si echec lors de l'upload : erreur fatale
		
		if(NNS_NC_OpenFile($ncfile,$ncarray)){
			if(NNS_NC_ExtractData($ncarray)){ //Parse du NC
				if(!NNS_NC_GeneratePreview($ncarray,$ncfile,true)){ //Generation de l'aperçu
					$ncarray['fatal_error'] = "Echec lors de la generation de l'aperçu";
				}else{
					if(strtolower(end(explode('.',$ncarray['info']['preview_file'])))=='png'){ //ajout des infos png
						NNS_PNG_FillInfo($ncarray['info']['preview_file'],
						$title=$ncrealname,
						$description="$ncrealname preview",
						$software="$version . $copyright",
						$comment="$ncrealname\r\nPreview : ".date('r')."\r\n$version . $copyright"
						);
						NNS_PNG_PPM($ncarray['info']['preview_file'],$ncarray['info']['preview_ppm']);
					}
				}
			}
		}
		
		if($ncarray['fatal_error']===false){
			if($debug && !$run_from_cmd){
				echo "<div class='ncmenu'><a href='#' onclick='document.getElementById(\"nccontener\").style.display = \"block\";'>[ Afficher le code NC ]</a> - <a href='#' onclick='document.getElementById(\"nccontener\").style.display = \"none\";'>[ Masquer le code NC ]</a></div><div id='nccontener'>";
				print_r($ncarray['preview_data']);
				echo "<div class='ncmenu'><a href='#' onclick='document.getElementById(\"nccontener\").style.display = \"none\";'>[ Masquer le code NC ]</a></div></div><br/>";
			} //----------------DEBUG : Affichage le debug
			
			//------------------------- Rapport
			if($run_from_cmd){echo "\n\n\nRapport pour le fichier '{$cmdcolwarn}{$ncarray['info']['file']['filename']}{$cmdcolreset}':\n";
			}else{echo "<strong><u>Rapport pour le fichier '<span class='parsewarn'>{$ncarray['info']['file']['filename']}</span>':</u></strong><br/>";}
			
			if($run_from_cmd){echo "	Vitesse maximum des axes X et Y :     {$cmdcolwarn}{$ncarray['settings']['cnc']['speed_fast_XY']}{$cmdcolreset}mm/min\n";
			}else{echo "Vitesse maximum des axes X et Y : <span class='parsewarn'>{$ncarray['settings']['cnc']['speed_fast_XY']}mm/min</span><br/>";}
			
			if($run_from_cmd){echo "	Vitesse maximum de l'axe Z :          {$cmdcolwarn}{$ncarray['settings']['cnc']['speed_fast_Z']}{$cmdcolreset}mm/min\n";
			}else{echo "Vitesse maximum de l'axe Z : <span class='parsewarn'>{$ncarray['settings']['cnc']['speed_fast_Z']}mm/min</span><br/>";}
			/*
			if($run_from_cmd){echo "	Dernier mode de coordonnees detecte : {$cmdcolwarn}";
			}else{echo "Dernier mode de coordonnees detecte : <span class='parsewarn'>";}
			
			if($nc_absolute){echo "Absolue (G90)";}else{echo "Relatif (G91)";}
			
			if($run_from_cmd){echo "{$cmdcolreset}\n\n\n";
			}else{echo "</span><br/><br/><br/>";}
			*/
			if($run_from_cmd){echo "\n\n";
			}else{echo "<br/><br/>";}
			
			if($run_from_cmd){echo "Rapport de temps:\n";
			}else{echo "<strong><u>Rapport de temps:</u></strong><br/>";}
			
			if($run_from_cmd){echo "	Temps en avance travail : {$cmdcolcomm}".NNS_sec2str($ncarray['info']['nc_times']['work'])."{$cmdcolreset}\n";
			}else{echo "Temps en avance travail : <span class='parsecomm'>".NNS_sec2str($ncarray['info']['nc_times']['work'])."</span><br/>";}
			
			if($run_from_cmd){echo "	Temps en avance rapide :  {$cmdcolcomm}".NNS_sec2str($ncarray['info']['nc_times']['fast'])."{$cmdcolreset}\n";
			}else{echo "Temps en avance rapide : <span class='parsecomm'>".NNS_sec2str($ncarray['info']['nc_times']['fast'])."</span><br/>";}
			
			if($run_from_cmd){echo "	Temps Total Theorique :   {$cmdcolcomm}".NNS_sec2str($ncarray['info']['nc_times']['total'])."{$cmdcolreset}\n";
			}else{echo "Temps Total Theorique : <span class='parseok'><u>".NNS_sec2str($ncarray['info']['nc_times']['total'])."</u></span><br/>";}
			
			for($i=0;$i<count($ncarray['info']['nc_operations']);$i++){
				if($ncarray['info']['nc_operations'][$i]['total'] != 0){
					if($run_from_cmd){
						echo "		'{$cmdcolcomm}{$ncarray['info']['nc_operations'][$i]['comment']}{$cmdcolreset}' : Travail : {$cmdcolcomm}".NNS_sec2str($ncarray['info']['nc_operations'][$i]['work'])."{$cmdcolreset} - Rapide : {$cmdcolcomm}".NNS_sec2str($ncarray['info']['nc_operations'][$i]['fast'])."{$cmdcolreset}\n";
					}else{echo "Temps pour '<span class='parsecomm'><u>{$ncarray['info']['nc_operations'][$i]['comment']}</u></span>' : Travail : <span class='parsecomm'>".NNS_sec2str($ncarray['info']['nc_operations'][$i]['work'])."</span> - Rapide : <span class='parsecomm'>".NNS_sec2str($ncarray['info']['nc_operations'][$i]['fast'])."</span><br/>";}
				}
			}
			
			if($run_from_cmd){echo "\n\n";
			}else{echo "<br/>";}
			
			$speed_percent = $ncarray['settings']['cnc']['speed_percent'];
			if($speed_percent<100){ //si un pourcentage d'avance < 100% defini
				if($run_from_cmd){echo "	Temps en avance travail (Avance {$cmdcolwarn}$speed_percent%{$cmdcolreset}) : {$cmdcolcomm}".NNS_sec2str($ncarray['info']['nc_times_percent']['work'])."{$cmdcolreset}\n";
				}else{echo "Temps en avance travail (Avance <span class='parsewarn'>$speed_percent%</span>) : <span class='parsecomm'>".NNS_sec2str($ncarray['info']['nc_times_percent']['work'])."</span><br/>";}
				
				if($run_from_cmd){echo "	Temps en avance rapide (Avance {$cmdcolwarn}$speed_percent%{$cmdcolreset}) :  {$cmdcolcomm}".NNS_sec2str($ncarray['info']['nc_times_percent']['fast'])."{$cmdcolreset}\n";
				}else{echo "Temps en avance rapide (Avance <span class='parsewarn'>$speed_percent%</span>) : <span class='parsecomm'>".NNS_sec2str($ncarray['info']['nc_times_percent']['fast'])."</span><br/>";}
				
				if($run_from_cmd){echo "	Temps Total Theorique (Avance {$cmdcolwarn}$speed_percent%{$cmdcolreset}) :   {$cmdcolok}".NNS_sec2str($ncarray['info']['nc_times_percent']['total'])."{$cmdcolreset}\n";
				}else{echo "Temps Total Theorique (Avance <span class='parsewarn'>$speed_percent%</span>) : <span class='parseok'><u>".NNS_sec2str($ncarray['info']['nc_times_percent']['total'])."</u></span><br/>";}
				
				for($i=0;$i<count($ncarray['info']['nc_operations']);$i++){
					if($ncarray['info']['nc_operations'][$i]['total'] != 0){
						$ncarray['info']['nc_operations'][$i]['work'] = $ncarray['info']['nc_operations'][$i]['work']/($speed_percent/100);
						$ncarray['info']['nc_operations'][$i]['fast'] = $ncarray['info']['nc_operations'][$i]['fast']/($speed_percent/100);
						if($run_from_cmd){
							echo "		'{$cmdcolcomm}{$ncarray['info']['nc_operations'][$i]['comment']}{$cmdcolreset}' ({$cmdcolwarn}$speed_percent%{$cmdcolreset}) : Travail : {$cmdcolcomm}".NNS_sec2str($ncarray['info']['nc_operations'][$i]['work'])."{$cmdcolreset} - Rapide : {$cmdcolcomm}".NNS_sec2str($ncarray['info']['nc_operations'][$i]['fast'])."{$cmdcolreset}\n";
						}else{echo "Temps pour '<span class='parsecomm'><u>{$ncarray['info']['nc_operations'][$i]['comment']}</u></span>' (Avance <span class='parsewarn'>$speed_percent%</span>) : Travail : <span class='parsecomm'>".NNS_sec2str($ncarray['info']['nc_operations'][$i]['work'])."</span> - Rapide : <span class='parsecomm'>".NNS_sec2str($ncarray['info']['nc_operations'][$i]['fast'])."</span><br/>";}
					}
				}
				
				if($run_from_cmd){echo "\n\n";
				}else{echo "<br/><br/>";}
			}
			
			if($run_from_cmd){echo "Apercu du programme:\n";
			}else{echo "<strong><u>Apercu du programme:</u></strong><br/>";}
			
			if(!$run_from_cmd){
			echo "<img src='{$ncarray['info']['preview_file']}' border='0'/><br/>";}
			
			if($run_from_cmd){echo "	Extremes X : {$cmdcolcomm}{$ncarray['info']['nc_limits']['x_min']}mm{$cmdcolreset} , {$cmdcolcomm}{$ncarray['info']['nc_limits']['x_max']}mm{$cmdcolreset} ({$cmdcolok}{$ncarray['info']['nc_limits']['x']}mm{$cmdcolreset})\n";
			}else{echo "Extremes X : <span class='parsecomm'>{$ncarray['info']['nc_limits']['x_min']}mm</span> , <span class='parsecomm'>{$ncarray['info']['nc_limits']['x_max']}mm</span> (<span class='parseok'>{$ncarray['info']['nc_limits']['x']}mm</span>)<br/>";}
			
			if($run_from_cmd){echo "	Extremes Y : {$cmdcolcomm}{$ncarray['info']['nc_limits']['y_min']}mm{$cmdcolreset} , {$cmdcolcomm}{$ncarray['info']['nc_limits']['y_max']}mm{$cmdcolreset} ({$cmdcolok}{$ncarray['info']['nc_limits']['y']}mm{$cmdcolreset})\n";
			}else{echo "Extremes Y : <span class='parsecomm'>{$ncarray['info']['nc_limits']['y_min']}mm</span> , <span class='parsecomm'>{$ncarray['info']['nc_limits']['y_max']}mm</span> (<span class='parseok'>{$ncarray['info']['nc_limits']['y']}mm</span>)<br/>";}
			
			if($run_from_cmd){echo "	Extremes Z : {$cmdcolcomm}{$ncarray['info']['nc_limits']['z_min']}mm{$cmdcolreset} , {$cmdcolcomm}{$ncarray['info']['nc_limits']['z_max']}mm{$cmdcolreset} ({$cmdcolok}{$ncarray['info']['nc_limits']['z']}mm{$cmdcolreset})\n";
			}else{echo "Extremes Z : <span class='parsecomm'>{$ncarray['info']['nc_limits']['z_min']}mm</span> , <span class='parsecomm'>{$ncarray['info']['nc_limits']['z_max']}mm</span> (<span class='parseok'>{$ncarray['info']['nc_limits']['z']}mm</span>)<br/>";}
			
			if($run_from_cmd){echo "\n\n";
			}else{echo "<br/><br/>";}
			
			if($run_from_cmd){echo "Rapport de chemin d'outil:\n";
			}else{echo "<strong><u>Rapport de chemin d'outil:</u></strong><br/>";}
			
			if($run_from_cmd){echo "	Distance en avance travail :                              {$cmdcolcomm}{$ncarray['info']['nc_travels']['work']}mm{$cmdcolreset}\n";
			}else{echo "Distance en avance travail : <span class='parsecomm'>{$ncarray['info']['nc_travels']['work']}mm</span><br/>";}
			
			if($run_from_cmd){echo "	Distance en avance rapide :                               {$cmdcolcomm}{$ncarray['info']['nc_travels']['fast']}mm{$cmdcolreset}\n";
			}else{echo "Distance en avance rapide : <span class='parsecomm'>{$ncarray['info']['nc_travels']['fast']}mm</span><br/>";}
			
			if($run_from_cmd){echo "	Distance en avance travail par interpolation circulaire : {$cmdcolcomm}{$ncarray['info']['nc_travels']['radius']}mm{$cmdcolreset}\n";
			}else{echo "Distance en avance travail par interpolation circulaire : <span class='parsecomm'>{$ncarray['info']['nc_travels']['radius']}mm</span><br/>";}
			
			if($run_from_cmd){echo "	Distance en cycle de percage :                            {$cmdcolcomm}{$ncarray['info']['nc_travels']['drill']}mm{$cmdcolreset}\n";
			}else{echo "Distance en cycle de perçage : <span class='parsecomm'>{$ncarray['info']['nc_travels']['drill']}mm</span><br/>";}
			
			if($run_from_cmd){echo "	Distance Total Theorique :                                {$cmdcolok}{$ncarray['info']['nc_travels']['total']}mm{$cmdcolreset}\n";
			}else{echo "Distance Total Theorique : <span class='parseok'><u>{$ncarray['info']['nc_travels']['total']}mm</u></span><br/>";}
			
			if($run_from_cmd){echo "\n\n";
			}else{echo "<br/><br/>";}
			
			if($run_from_cmd){echo "Rapport relatif au code NUM:\n";
			}else{echo "<strong><u>Rapport relatif au code NUM:</u></strong><br/>";}
			
			if($run_from_cmd){echo "	Lignes dans le programme : {$cmdcolcomm}{$ncarray['info']['nc_lines']['nc']}{$cmdcolreset}\n";
			}else{echo "Lignes dans le programme : <span class='parsecomm'>{$ncarray['info']['nc_lines']['nc']}</span><br/>";}
			
			if($run_from_cmd){echo "	Lignes de commentaire :    {$cmdcolcomm}{$ncarray['info']['nc_lines']['comment']}{$cmdcolreset}\n";
			}else{echo "Lignes de commentaire : <span class='parsecomm'>{$ncarray['info']['nc_lines']['comment']}</span><br/>";}
			
			if($run_from_cmd){echo "	Lignes ignorees :          {$cmdcolcomm}{$ncarray['info']['nc_lines']['ignored']}{$cmdcolreset}\n";
			}else{echo "Lignes ignorees : <span class='parsecomm'>{$ncarray['info']['nc_lines']['ignored']}</span><br/>";}
			
			if($run_from_cmd){echo "	Ligne en deplacement G0 :  {$cmdcolcomm}{$ncarray['info']['nc_lines']['g0']}{$cmdcolreset}\n";
			}else{echo "Ligne en deplacement G0 : <span class='parsecomm'>{$ncarray['info']['nc_lines']['g0']}</span><br/>";}
			
			if($run_from_cmd){echo "	Ligne en deplacement G1 :  {$cmdcolcomm}{$ncarray['info']['nc_lines']['g1']}{$cmdcolreset}\n";
			}else{echo "Ligne en deplacement G1 : <span class='parsecomm'>{$ncarray['info']['nc_lines']['g1']}</span><br/>";}
			
			if($run_from_cmd){echo "	Ligne en deplacement G2 :  {$cmdcolcomm}{$ncarray['info']['nc_lines']['g2']}{$cmdcolreset}\n";
			}else{echo "Ligne en deplacement G2 : <span class='parsecomm'>{$ncarray['info']['nc_lines']['g2']}</span><br/>";}
			
			if($run_from_cmd){echo "	Ligne en deplacement G3 :  {$cmdcolcomm}{$ncarray['info']['nc_lines']['g3']}{$cmdcolreset}\n";
			}else{echo "Ligne en deplacement G3 : <span class='parsecomm'>{$ncarray['info']['nc_lines']['g3']}</span><br/>";}
			
			if($run_from_cmd){echo "	Cycle de percage G81 G83 : {$cmdcolcomm}{$ncarray['info']['nc_lines']['g81-g83']}{$cmdcolreset}\n";
			}else{echo "Cycle de perçage G81 G83 : <span class='parsecomm'>{$ncarray['info']['nc_lines']['g81-g83']}</span><br/>";}
			
			if($run_from_cmd){echo "\n\n";
			}else{echo "<br/><br/>";}
			
			if($run_from_cmd){echo "Temps de generation de l'apecu : {$cmdcolcomm}".round($ncarray['info']['compute_time']['generatepreview'],4)."sec{$cmdcolreset}\n";
			}else{echo "Temps de generation de l'apecu : <span class='parsecomm'>".round($ncarray['info']['compute_time']['generatepreview'],4)."sec</span><br/>";}
			
			if($run_from_cmd){echo "Temps de traitement du code NC : {$cmdcolcomm}".round($ncarray['info']['compute_time']['extractdata'],4)."sec{$cmdcolreset}\n";
			}else{echo "Temps de traitement du code NC : <span class='parsecomm'>".round($ncarray['info']['compute_time']['extractdata'],4)."sec</span><br/>";}
		}else{
			if($run_from_cmd){
				echo "{$cmdcolerror}{$ncarray['fatal_error']}{$cmdcolreset}\n";
			}else{
				echo "<div class='ncmenu'><strong><u>/!\\ /!\\ /!\\ /!\\ /!\\</u></strong><br/><span class='parseerror'>{$ncarray['fatal_error']}</span></div>";
			}
		}
	}
	
	if($run_from_cmd){
		echo "\n\nFin de rapport...";
		$line = fgets($cmdhandle);
	}else{
		echo "<br/><br/><div class='ncmenu'>$version - $copyright<div class='ncmenu'></body></html>";
	}
}
?>