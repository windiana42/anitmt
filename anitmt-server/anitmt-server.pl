#!/usr/bin/perl -w

# ----------------------------------------------------------------------------
# name:		anitmt-server.pl - the anitmt server system
# task:		runs the anitmt server for distributed rendering of TCP/IP
#		networks
#		the clinet ist anitmt-client, current version 0.01
# language:	Perl (developed using perl 5.005_03 on a linux box)
# license:	GPL (General Public Lisence), see file license
# maintainer:	Jan Theofel, jan@theofel.de
# version:	s0.04, build 2001-07-02-sql
# new versions:	http://www.theofel.de/anitmt/
# ----------------------------------------------------------------------------
# important notes: -PLEASE READ CAREFULLY IF YOU WANT TO CHANGE THE SOURCE-
# 
# [1] The complete source is described in 'anitmt hacker's guide part II
#     networking'. Please read this if your intereted in the source before
#     modifing it!
# ---------------------------------------------------------------------------- 


# ---------- including the libs ----------
use Ssockets;
use FileHandle;
use Socket;
use Getopt::Long;
# use IPC::Shareable;
use Time::localtime;
use DBI;

# ---------- main program ----------
&init;
&run;

sub init
# ----------------------------------------------------------------------------
# task:		initialises the server
# author:	Jan Theofel, jan@theofel.de
# last changed: 2001-07-01
# ----------------------------------------------------------------------------
{
  &set_const;
  &init_vars;
  &init_sql;
#  &restore_clients;    WE DON'T NEED THAT FOR SQL
  &check_params;
  &set_handlers;
  &open_socket;
}

sub init_sql
# ----------------------------------------------------------------------------
# task:		inits the SQL connection and logs the projects, clients, ...
#               which are already in the database
# author:	Jan Theofel, jan@theofel.de
# last changed: 2001-07-01
# ----------------------------------------------------------------------------
{
  my $dbserver = "DBI:mysql:anitmtserver:localhost";
  my $dbuser = "anitmtserver";
  my $dbpass = "anitmtserver";

  $dbh = DBI->connect($dbserver,$dbuser,$dbpass) || die("Verbindung zum MySQL Server fehlgeschlagen."); 

  print "DEBUG: Checking SQL database for existing projects...\n";
  my $sth = $dbh->prepare("SELECT name, author, comment FROM projects");
  my $ok = $sth->execute;
  while ((my ($name, $author, $comment) = $sth->fetchrow_array))
  {
    print "       project: $name; author: $author; comment: $comment\n";
  }
  $sth->finish;
}

sub init_vars
# ----------------------------------------------------------------------------
# task:		inits the variables
# author:	Jan Theofel, jan@theofel.de
# last changed: 2001-07-01
# ----------------------------------------------------------------------------
{

  $parent_pid = $$;

#  tie %clients, 'IPC::Shareable', undef, { destroy => 0 } || die "Tie failed!\n"; 
#  tie %projects, 'IPC::Shareable', undef, { destroy => 0 } || die "Tie failed\n";

#  ALL USAGED OF THIS WHERE REMOVED: 
#  tie %client_pids, 'IPC::Shareable', undef, { destroy => 0 } || die "Tie failed!\n";

  # ----------- command sets -----------
  %default_commands =
  ( "HELO" => 1,
    "EHLO" => 0,
    "INFO" => 0,
    "BYE"  => 1 ) ;

  # ---------- the rif informations ---------
  %rif =
  ( "FPS"		=> 24,
    "FRAMES"		=> 1,
    "HEIGHT"		=> 320,
    "WIDTH"		=> 200,
    "RAYTRACER"		=> "POVRAY3.1",
    "PARAMS"    	=> "",
    "FRAME_SRC" 	=> "F%4.INC",
    "FRAME_DEST"	=> "FRAME.INC",
    "MAIN_FILE"		=> "POVRAY31.POV"
  );
}

# sub restore_clients    !!! NOT NEEDED FOR SQL ANY MORE
# ----------------------------------------------------------------------------
# task:		restores the clients from the backup file
# author:	Jan Theofel, jan@theofel.de
# last changed: 2000-02-23
# ----------------------------------------------------------------------------
#{
#
#  if(-e $client_info_filename)
#  {
#    server_log("Client backup file $client_info_filename is found. Reading...\n");
#    if(open(BACKUP,"<$client_info_filename"))
#    {
#      (tied %clients)->shlock();
#      
#      server_log("...skipping first three line!\n");
#      $skip_line = <BACKUP>;
#      $skip_line = <BACKUP>;
#      $skip_line = <BACKUP>;
#
#      while(<BACKUP>)
#      {
#        $line = $_; chomp($line);
#        $line =~ s/^CID=//;
#        $local_cid = $line;
#        $line = <BACKUP>;chomp($line);
#        $line =~ s/^SYS=//;
#        $local_client{SYS} = $line;
#        $line = <BACKUP>;chomp($line);
#        $line =~ s/^SPED_SIZE=//;
#        $local_client{SPED_SIZE} = $line;
#        $line = <BACKUP>;chomp($line);
#        $line =~ s/^SPED_UNIT=//;
#        $local_client{SPED_UNIT} = $line;
#        $line = <BACKUP>;chomp($line);
#        $line =~ s/^JOBC_SIZE=//;
#        $local_client{JOBC_SIZE} = $line;
#        $line = <BACKUP>;chomp($line);
#        $line =~ s/^JOBC_UNIT=//;
#        $local_client{JOBC_UNIT} = $line;
#        $line = <BACKUP>;chomp($line);
#        $line =~ s/^RESC_SIZE=//;
#        $local_client{RESC_SIZE} = $line;
#        $line = <BACKUP>;chomp($line);
#        $line =~ s/^RESC_UNIT=//;
#        $local_client{RESC_UNIT} = $line;
#        $line = <BACKUP>;chomp($line);
#        $line =~ s/^RR=//;chomp($line);
#        @local_client_rr = split(/,/,$line);
#        $line = <BACKUP>;chomp($line);
#        $line =~ s/^FP=//;
#        @local_client_fp = split(/,/,$line);
#        $line = <BACKUP>;chomp($line);
#        $line =~ s/^DEXM=//;
#        $local_client{DEXM} = $line;
#        $line = <BACKUP>;chomp($line);
#        $line =~ s/^IP=//;
#        $local_client{IP} = $line;
#        $line = <BACKUP>;chomp($line);
#        $line =~ s/^PID=//;
#        $local_client{PID} = $line;
#        $line = <BACKUP>;chomp($line);
#        $line =~ s/^CONNECTED=//;
#        $local_client{CONNECTED} = $line;
#        $line = <BACKUP>;chomp($line);
#        $line =~ s/^STATUS=//;
#        $local_client{STATUS} = $line;
#        $skip_line = <BACKUP>;chomp($line);       
#      
#        $clients{$local_cid} = {
#          SYS 		=> $local_client{SYS}, 
#          SPED_SIZE 	=> $local_client{SPED_SIZE},
#          SPED_UNIT 	=> $local_client{SPED_UNIT},
#          JOBC_SIZE 	=> $local_client{JOBC_SIZE},
#          JOBC_UNIT 	=> $local_client{JOBC_UNIT},
#          RESC_SIZE 	=> $local_client{RESC_SIZE},
#          RESC_UNIT 	=> $local_client{RESC_UNIT},
#          RR		=> [ @local_client_rr ],
#          FP		=> [ @local_client_fp ],
#          DEXM		=> $local_client{DEXM},
#          IP		=> $local_client{IP},
#          PID		=> 0,
#          CONNECTED	=> 0,
#	  STATUS	=> $local_client{STATUS}
#        };
#      
#      }
#      (tied %clients)->shunlock();
#      close(BACKUP);
#    }
#    else
#    {
#      server_log("Can't open file for reading! Check permissions please.");
#    }
#  }
#  else
#  {
#    server_log("No client backup file $client_info_filename. Starting without clients.\n");
#  }
#}

sub set_const
# ----------------------------------------------------------------------------
# task:		defines some consts
# author:	Jan Theofel, jan@theofel.de
# last changed: 2001-07-01
# ----------------------------------------------------------------------------
{

  # ---------- some default values of the vars ---------
  $server_port = 4004;

  $server_version = "s0.04";
  $server_build = "2001-07-02-sql";
  $protocoll_version = "p0.01";
  $identify = "anitmtserver $server_version (build $server_build)";

  # ---------- names to the logfiles ----------
  $protocoll_log = "protocoll.log";
			# where to log the complete protocoll
  $server_log = "server.log";
			# where to log the server messages
  $error_log = "error.log";
			# where to store the error messages
#  $client_info_filename = "clients.backup";
  $default_out_path = "/tmp/anitmt/server-output/";

  # ---------- set the server messages ---------
  $ret_good_bye 	
    = "900 CU L8ER!";
  $ret_130	
    = "130 MAL FOMRED COMMAND! TRY 'MSG' OR 'HELP'.";
  $ret_140
    = "140 COMMAND NOT ALLOWED HERE! TRY 'MSG' OR 'HELP'.";
  $ret_180
    = "180 VALUE ACCEPTED!";
  $ret_190
    = "190 VALUE DENIED! TRY 'MSG' OR 'HELP'.";

  $ret_debug_help =
    "811 DEBUG HELP\n" .
    "811 ----------\n" .
    "811 Allowed command are:\n".
    "811 debug off              - switches debug mode off again\n" .
    "811 debug info             - general server system informations\n" .
    "811 debug pids             - sends server and client pids\n" .
    "811 debug rif              - information about rif file\n" .
    "811 debug projects		- lists all jobs\n" .
    "811 debug clients detailed - detailed list of all known clients\n" .
    "811 For more informations read 'anitmt - networking hacker's guide at\n" .
    "812   http://www.theofel.de/anitmt/";

  $dexm_offer_nfs	= 1;
  $dexm_offer_ftp	= 0;
  $dexm_offer_internal	= 0;

  $dexm_nfs_path	= "/tmp/anitmt/dexm-nfs/";
  $dexm_nfs_path_in	= $dexm_nfs_path . "in-data/";
  $dexm_nfs_path_out	= $dexm_nfs_path . "out-data/";

  # --- const which define the dexm mode for the clients ---
#  $const_dexm_none 	= 0;
#  $const_dexm_internal	= 1;  
#  $const_dexm_ftp	= 2;
#  $const_dexm_nfs	= 4;

  # --- client status codes ---
  $const_clstatus_none		= 0;		# some error status
  $const_clstatus_talking	= 1;		# somewhere in the protocoll
						# i.e. help/debug
  $const_clstatus_created	= 2;		# client just created 
  $const_clstatus_rendering	= 4;		# rendering (offline!)
  $const_clstatus_get_data	= 8;		# client is getting data
  $const_clstatus_send_data	= 16;		# client sends data
}

sub set_handlers
# ----------------------------------------------------------------------------
# task:		sets the hendlers to act on system signals
# author:	Jan Theofel, jan@theofel.de
# last changed: 2000-02-20
# ----------------------------------------------------------------------------
{
  $SIG{'INT'} = $SIG{'QUIT'} = \&exit_request_handler;
  $SIG{'CHLD'} = \&child_handler;
  $SIG{'HUP'} = \&signal_sighup;
}

sub open_socket
# ----------------------------------------------------------------------------
# task:		opens a 'listen' socket
# author:	Jan Theofel, jan@theofel.de
# last changed: 2000-02-20
# ----------------------------------------------------------------------------
{
  unless(listensocket(*SERVERSOCKET, $server_port, 'tcp', 5))
  {
    error_log("$0: " . $Ssockets::error);
    die;
  }

  autoflush SERVERSOCKET 1;

  server_log("anitmt server started, using port $server_port\n");

}

sub run
# ----------------------------------------------------------------------------
#  task:	this is the 'heart' of the anitmt server script: it handles
#		the connections with the clients in an never ending loop
#		(the server can only be stopped by signals)
# last changed: 2000-01-02
# ----------------------------------------------------------------------------
{
  while(1)
  {

    # ---------- wait for an incomming connection ----------
    ACCEPT_CONNECT:
    {
      ($remaddr = accept(CHILDSOCKET, SERVERSOCKET)) || redo ACCEPT_CONNECT;
    }
    autoflush CHILDSOCKET 1;
    my $pid = fork();
    if (defined($pid))
    {
      server_log("started child with pid $pid\n");
    }
    else
    {
      error_log("Cannot fork, $!");
      die;
    }
    
    if ($pid == 0)
    {
##      $child_pid = $$;  INSERT IN SQL LATER AGAIN
      %allowed_commands = %default_commands;
      $allow_debug = 0;
      print CHILDSOCKET "001 $identify - protocoll version $protocoll_version\n";
      print CHILDSOCKET "001 PLEASE CONNET USING \"HELO\". \"BYE\" TO QUIT.\n";
      print CHILDSOCKET "002 YOU CAN GET HELP BY SENDING 'HELP' AS A COMMAND.\n";
      $remip = inet_ntoa((unpack_sockaddr_in($remaddr))[1]);
      server_log("Connection accepted from $remip by child $$\n");

#      STILL TO BE CHANGED FOR SQL      
#      (tied %client_pids)->shlock();
#      $client_pids{$child_pid} = { IP => $remip, LAST => "(logged in)" };
#      (tied %client_pids)->shunlock();

      while(<CHILDSOCKET>)
      {
        s/\012//;
        s/\015//;
        $cmdline = $_;
        pro_log("from $remip","$cmdline\n");
      
#      STILL TO BE CHANGED FOR SQL      
#        (tied %client_pids)->shlock();
#        $client_pids{$child_pid} = { IP => $remip, LAST => $cmdline };
#        (tied %client_pids)->shunlock();

        $orig_cmdline = $cmdline;
        $cmdline = uc($cmdline);
        $reply = &handle_command($cmdline);
        print CHILDSOCKET $reply . "\n";
        pro_log("to $remip", $reply . "\n");
        if($reply eq $ret_good_bye)
          { last; }
      }

      unless (close(CHILDSOCKET))
      {
        print "Couldn't close the child socket (parent), $!\n";
      }

#      STILL TO BE CHANGED FOR SQL      
#      (tied %client_pids)->shlock();                   
#      $client_pids{$child_pid} = { IP => $remip, LAST => "(died)" };
#      (tied %client_pids)->shunlock();

      exit(0);

    }
  }
}

sub handle_command
# ----------------------------------------------------------------------------
# task:		handles the connection with one client
# author:	Jan Theofel, jan@theofel.de
# last changed: 2000-02-21
# ----------------------------------------------------------------------------
{

  my ($command) = (@_);
  my $ret = "";

  # ---------- BYE (last changed 2001-07-01) ---------------------------------
  if($command eq "BYE")
  {
    $ret = $ret_good_bye; 

    if(defined($cid))
    {
      print "DEBUG: CID $cid is set 'not connected'...\n"; 
#      # REFERENCE TO NOTE [2] AT THE BEGINNING!!!
#      (tied %clients)->shlock;
#      $clients{$cid} = {
#        SYS 		=> $clients{$cid}->{SYS}, 
#        SPED_SIZE 	=> $clients{$cid}->{SPED_SIZE},
#        SPED_UNIT 	=> $clients{$cid}->{SPED_UNIT},
#        JOBC_SIZE 	=> $clients{$cid}->{JOBC_SIZE},
#        JOBC_UNIT 	=> $clients{$cid}->{JOBC_UNIT},
#        RESC_SIZE 	=> $clients{$cid}->{RESC_SIZE},
#        RESC_UNIT 	=> $clients{$cid}->{RESC_UNIT},
#        RR		=> [ @{$clients{$cid}->{RR}} ],
#        FP		=> [ @{$clients{$cid}->{FP}} ],
#        DEXM		=> $clients{$cid}->{DEXM},
#        IP		=> $remip,
#        PID		=> $$,
#        CONNECTED	=> 0,
#        STATUS		=> $clients{$cid}->{STATUS} 
#      };
#      (tied %clients)->shunlock;

      $sql = "UPDATE clients SET connected=0 WHERE cid='$cid'";
      my $change_data = $dbh->prepare($sql);
      $change_data->execute;
 
#     print "DEBUG information follows:
#      foreach $key (keys %{$clients{$cid}})
#      {
#        print "$key is now $clients{$cid}->{$key}\n";
#      }
    }
    else
    {
      print "DEBUG: Connection closed via 'BYE': CID not defined. Can't disconnect!\n";
    }
  }
  # ---------- HELO (last changed 2000-01-23) --------------------------------
  elsif($command =~ /^HELO(.*)/)	# last changed HELO: 2000-01-23
  { 
    if($allowed_commands{HELO} == 1) 
    {
      if($1 eq "")
      {
        $ret = "100 HELO $remip";
        $msg = "Next commands can be SEHE or INFO.";
        $allowed_commands{HELO} = 0;
        $allowed_commands{SEHE} = 1;
        $allowed_commands{INFO} = 1;
      }
      else
      {
        $ret = $ret_130;
        $msg = "HELO doesn't take any additional parameters";
      }
    }
    else
    {
      $ret = $ret_140;
      $msg = "You are already logged in."; 
    }
  }
  # ---------- SEHE (last changed 2001-07-01) --------------------------------
  elsif($command =~ /^SEHE\s*(.*)/)
  {
    $cid = $1;
    if($allowed_commands{SEHE} == 1)
    {
      # --- do we have such a client? ---
      my $sth = $dbh->prepare("SELECT connected, connected_from FROM clients WHERE cid='$cid'");
      my $ok = $sth->execute;
      my $found_client = ((my ($client_connected, $client_connected_from) = $sth->fetchrow_array));
      $sth->finish;
      server_log("SEHE DEBUG INFO: found: $found_client, connected: $client_connected, from: $client_connected_from\n");

#      foreach $key (keys %clients)
#      {
#        if ($key eq $cid)
#        {
#          $found_client = 1;
#        }
#      }
      
      # --- send some messages back ---
      if(!$found_client)
      {
        $ret = "190 SEHE DENIED! NO SUCH CID.";
        $msg = "SEHE command have send $cid. There is no such cid in my list.";
      }
      elsif($client_connected)
      {
        $ret = "190 SEHE DENIED! ALREADY CONNECTED FROM $client_connected_from";
        $msg = "This cid is already connected from the IP $client_connected_from, possiblly started twice?";
      }
      else
      {
        $ret = "101 SEHE $remip AS $cid\n101 YOUR LAST LOGIN WAS FROM $client_connected_from\n102 ACCEPTED COMMANDS ARE ALL EXPECT LOGIN.";
        $allowed_commands{HELO} = 0;
        $allowed_commands{SEHE} = 0;
        $allowed_commands{INFO} = 0;

        $allowed_commands{GET} = 1;
        $allowed_commands{SEND} = 1;
      
        # REFERENCE TO NOTE [2] AT THE BEGINNING!!!
#        (tied %clients)->shlock;
#        $clients{$cid} = {
#          SYS 		=> $clients{$cid}->{SYS}, 
#          SPED_SIZE 	=> $clients{$cid}->{SPED_SIZE},
#          SPED_UNIT 	=> $clients{$cid}->{SPED_UNIT},
#          JOBC_SIZE 	=> $clients{$cid}->{JOBC_SIZE},
#          JOBC_UNIT 	=> $clients{$cid}->{JOBC_UNIT},
#          RESC_SIZE 	=> $clients{$cid}->{RESC_SIZE},
#          RESC_UNIT 	=> $clients{$cid}->{RESC_UNIT},
#          RR		=> [ @{$clients{$cid}->{RR}} ],
#          FP		=> [ @{$clients{$cid}->{FP}} ],
#          DEXM		=> $clients{$cid}->{DEXM},
#          IP		=> $remip,
#          PID		=> $$,
#          CONNECTED	=> 1,
#	  STATUS	=> $const_clstatus_none
#        };
#        (tied %clients)->shunlock;

        $sql = "UPDATE clients SET connected=1, connected_from='$remip' WHERE cid='$cid'";
        my $change_data = $dbh->prepare($sql);
        $change_data->execute;
 
      }
    }
    else
    {
      $ret = "141 'SEHE' IS NOT ALLOWED.\n142 YOU COULD NEED 'HELP'.";
    }
  }
  # ---------- INFO (last changed 2000-01-23) --------------------------------
  elsif($command =~ /^INFO(\.*)/)
  {
    if($allowed_commands{INFO} == 1)
    {
      if($1 eq "")
      {
        $ret = "100 OK TO SEND INFO FOR $remip";
        $msg = "Next command must be SYS.";
        $allowed_commands{HELO} = 0;
        $allowed_commands{SEHE} = 0;
        $allowed_commands{INFO} = 0;
        $allowed_commands{SYS} = 1;
      
        # --- setting of some default values ---
        @this_client_rr 		= ();
        @this_client_fp 		= ();
        $this_client{SYS} 		= "unknown";
        $this_client{SPED_SIZE} 	= 0;
        $this_client{SPED_UNIT} 	= "NRPMIPS";
        $this_client{JOBC_SIZE} 	= 0;
        $this_client{JOBC_UNIT} 	= "mB";
        $this_client{RESC_SIZE} 	= 0;
        $this_client{RESC_UNIT} 	= "mB";
        $this_client{DEXM_INTERNAL}	= 0;
        $this_client{DEXM_FTP}		= 0;
        $this_client{DEXM_COPY}		= 0;
        $this_client{CONNECTED}		= 1;
	$this_client{STATUS}		= $const_clstatus_none;
      }
      else
      {
        $ret = $ret_130;
        $msg = "INFO doesn't accept any additional parameters.";
      }
    }
    else
    {
      $ret = $ret_140;
      $msg = "INFO is not allowed except after HELO.";
    }
  }
  # ---------- SYS (last changed 2000-01-23) ---------------------------------
  elsif($command =~ /^SYS\s*(.*)/)
  {
    my $sys = $1;
    if($allowed_commands{SYS} == 1)
    {
      if($1 eq "")
      {
        $ret = $ret_130;
	$msg = "SYS needs a string parameter";
      }
      else
      {
        $ret = $ret_180;
        $msg = "Next command must be 'SPED'.";
        $allowed_commands{SYS} = 0;
        $allowed_commands{SPED} = 1;
        $this_client{SYS} = $sys;
      }
    }
    else
    {
      $ret = $ret_140;
      $msg = "SYS is only allowed directly after INFO.";
    }
  }
  # ---------- SPED (last changed 2000-01-23) --------------------------------
  elsif($command =~ /^SPED\s*(\S*)\s*(\S*)/)
  {
    if($allowed_commands{SPED} == 1)
    {
      $param1 = $1;
      $param2 = $2;
      if($param1 eq "")
      {
        $ret = $ret_130;
        $msg = "SPED needs two parameters: value and unit";
      }
      elsif($param2 eq "")
      {
        $ret = $ret_130;
        $msg = "SPED needs two parameters: value and unit";
      }
      else
      {
        if($param1 !~ /\d*/)
        { 
          $ret = $ret_130;
          $msg = "First SPED parameter must be a number.";
        }
        if($param2 ne "NRPMIPS")
        {
          $ret = $ret_130;
          $msg = "SPED unit must be NRPMIPS at the moment.";
        }
        else
        {
          $ret = $ret_180; 
          $msg = "Next command must be JOBC.";
          $allowed_commands{JOBC} = 1;
          $allowed_commands{SPED} = 0;
          $this_client{SPED_SIZE} = $param1;
          $this_client{SPED_UNIT} = $param2;
        }
      }
    }
    else
    {
      $ret = $ret_140; 
      $msg = "SPED is only allowed after SYS.";
    }
  }

  # ---------- JOBCACHE ------------------------------------------------------
  elsif($command =~ /^JOBC\s*(\S*)\s*(\S*)/)
  {
    $param1 = $1;
    $param2 = $2;
    if($allowed_commands{JOBC} == 1)
    {
      if($param1 eq "")
      {
        $ret = $ret_130;
        $msg = "JOBC needs two parameters: size and unit [b, kB, MB, GB]";
      }
      elsif($param2 eq "")
      {
        $ret = $ret_130;
        $msg = "JOBC needs two parameters: size and unit [b, kB, MB, GB]";
      }
      else
      {
        if($param1 !~ /\d*/)
        {
          $ret = $ret_130;
          $msg = "First JOBC parameter must be a number.";
        }
        elsif(!($param2 eq "B"||$param2 eq "KB"||$param2 eq "MB"||$param2 eq "GB"))
        {
          $ret = $ret_130;
          $msg = "Second JOBC parameter must be the unit [b, kB, MB, GB]";
        }
        else
        {
          $ret = $ret_180;
          $msg = "Next command must be RESC.";
          $allowed_commands{JOBC} = 0;
          $allowed_commands{RESC} = 1;
          $this_client{JOBC_SIZE} = $param1;
          $this_client{JOBC_UNIT} = $param2;
        }
      }
    }
    else
    {
      $ret = $ret_140;
      $msg = "JOBC is only allowed after SPED.";
    }
  }

  # --------- RESULTCHACHE ---------------------------------------------------
  elsif($command =~ /^RESC\s*(\S*)\s*(\S*)/)
  {
    $param1 = $1;
    $param2 = $2;
    if($allowed_commands{RESC} == 1)
    {
      if($param1 eq "")
      {
        $ret = $ret_130;
        $msg = "RESC needs two parameters: size and unit [b, kB, MB, GB]";
      }
      elsif($param2 eq "")
      {
        $ret = $ret_130;
        $msg = "RESC needs two parameters: size and unit [b, kB, MB, GB]";
      }
      else
      {
        if($param1 !~ /\d*/)
        {
          $ret = $ret_130;
          $msg = "First RESC parameter must be a number.";
        }
        elsif(!($param2 eq "B"||$param2 eq "KB"||$param2 eq "MB"||$param2 eq "GB"))
        {
          $ret = $ret_130;
          $msg = "Second RESC parameter must be the unit [b, kB, MB, GB]";
        }
        else
        {
          $ret = $ret_180; 
          $allowed_commands{ADD_RR} = 1;
          $allowed_commands{LAST_RR} = 1;
          $allowed_commands{RESC} = 0;
          $this_client{RESC_SIZE} = $param1;
          $this_client{RESC_UNIT} = $param2;
        }
      }
    }
    else
    {
      $ret = $ret_140;
      $msg = "RESC ist only allowed after JOBC."; 
    }
  }

  # ---------- ADD RENERER / RAYTRACER ---------------------------------------
  elsif($command =~ /^ADRR\s*(.*)$/)
  {
    my $rr = $1;
    if($allowed_commands{ADD_RR} == 1)
    {
      $ret = "111 ADDED AN R/R FOR $remip: $rr\n112 SEND ANOTHER 'ADD_RR' OR 'LAST_RR'";
      $allowed_commands{ADD_RR} = 1;
      $allowed_commands{LAST_RR} = 1;
      push(@this_client_rr, $rr);
    }
    else
    {
      $ret = "141 'ADD_RR' IS NOT ALLOWED HERE.\n142 YOU COULD NEED 'HELP'.";
    }
  } 

  # ---------- LAST RENERER / RAYTRACER --------------------------------------
  elsif($command =~ /^LARR/)
  {
    if($allowed_commands{LAST_RR} == 1)
    {
      $ret = "111 NO MORE R/R FOR $remip\n112 SEND OUR F/P USING 'ADD_FP'";
      $allowed_commands{ADD_FP} = 1;
      $allowed_commands{LAST_FP} = 1;
      $allowed_commands{ADD_RR} = 0;
      $allowed_commands{LAST_RR} = 0;
    }
    else
    {
      $ret = "141 'LAST_RR' IS NOT ALLOWED HERE.\n142 YOU COULD NEED 'HELP'.";
    }
  }

  # ---------- ADD FILTER / PLUGIN  ------------------------------------------
  elsif($command =~ /^ADFP\s*(.*)$/)
  {
    my $fp = $1;
    if($allowed_commands{ADD_FP} == 1)
    {
      $ret = "101 ADDED AN F/P FOR $remip: $fp\n102 SEND ANOTHER 'ADD_FP' OR 'LAST_FP'";
      $allowed_commands{ADD_FP} = 1;
      $allowed_commands{LAST_FP} = 1;
      push(@this_client_fp, $fp);
    }
    else
    {
      $ret = "141 'ADD_RR' IS NOT ALLOWED HERE.\n142 YOU COULD NEED 'HELP'.";
    }
  }

  # ---------- LAST FILTER / PLUGIN  -----------------------------------------
  elsif($command =~ /^LAFP/)
  {
    if($allowed_commands{LAST_FP} == 1)
    {
      $ret = "111 NO MORE F/P FOR $remip\n112 SELECT DATA EXCHANGE MODE 'DEXM'";
      $allowed_commands{ADD_FP} = 0;
      $allowed_commands{LAST_FP} = 0;
      $allowed_commands{DEXM} = 1;
    }
    else
    {
      $ret = "141 'LAST_FP' IS NOT ALLOWED HERE.\n142 YOU COULD NEED 'HELP'.";
    }
  }

  # ---------- DATA EXCHANGE MODE (DEXM) -------------------------------------
  elsif($command =~ /^DEXM\s*(.*)/)
  {
    if($allowed_commands{DEXM} == 1)
    {
      if($1 eq "INTERNAL")
      { 
        if($dexm_offer_internal)
        {
          $ret = "180 VALUE ACCEPTED: \"INTERNAL\"\n";
          $allowed_commands{DEXM} = 0;
          $allowed_commands{FIIN} = 1;
          $this_client{DEXM_INTERNAL} = 1; 
        }
        else
        {
          $ret = $ret_190; 
          $msg = "Internal data exchange mode is not offerd by this server.";
          $allowed_commands{FIIN} = 0;
        }
      }
      elsif($1 eq "NFS")
      { 
        if($dexm_offer_nfs)
        {
          $ret = "180 VALUE ACCEPTED: \"NFS\"";
          $allowed_commands{DEXM} = 0;
          $allowed_commands{FIIN} = 1;
          $this_client{DEXM_COPY} = 1; 
        }
        else
        {
          $ret = $ret_190;   
          $msg = "NFS data exchange mode is not offerd by this server.";
          $allowed_commands{FIIN} = 0;
        }
      }
      elsif($1 eq "FTP")
      { 
        if($dexm_offer_ftp)
        {
          $ret = "180 VALUE ACCEPTED: \"FTP\"\n";
          $allowed_commands{DEXM} = 0;
          $allowed_commands{FIIN} = 1;
          $this_client{DEXM_FTP} = 1;
        }
        else
        {
          $ret = $ret_190; 
          $msg = "FTP data exchange mode is not offerd by this server.";
          $allowed_commands{FIIN} = 0;
        }
      }
      else
        { $ret = $ret_130; }
    }
    else
    {
      $ret = "141 'DEXM' IS NOT ALLOWED HERE.\n142 YOU COULD NEED 'HELP'.";
    }
  }

  # ---------- FINISH INFOMRATION --------------------------------------------
  elsif($command =~ /^FIIN/)
  {
    if($allowed_commands{FIIN} == 1)
    {
      $cid_time = time();
      $cid_to_send = $remip . "_" . $cid_time;
      $ret = "101 FINISHING FOR $remip\n101 YOUR CID: \"$cid_to_send\"\n102 PLEASE DISCONNECT NOW USING 'BYE' AND RETURN.";
      $allowed_commands{BYE} = 1;
      $allowed_commands{FIIN} = 0;

      $cid = $cid_to_send;		# for the other source parts

#      # REFERENCE TO NOTE [2] AT THE BEGINNING!!!
#      (tied %clients)->shlock;
#      $clients{$cid} = {
#        SYS 		=> $this_client{SYS}, 
#        SPED_SIZE 	=> $this_client{SPED_SIZE},
#        SPED_UNIT 	=> $this_client{SPED_UNIT},
#        JOBC_SIZE 	=> $this_client{JOBC_SIZE},
#        JOBC_UNIT 	=> $this_client{JOBC_UNIT},
#        RESC_SIZE 	=> $this_client{RESC_SIZE},
#        RESC_UNIT 	=> $this_client{RESC_UNIT},
#        RR		=> [ @this_client_rr ],
#        FP		=> [ @this_client_fp ],
#        DEXM		=> $this_client{DEXM},
#        IP		=> $remip,
#        PID		=> $child_pid,
#        CONNECTED	=> 0,
#	STATUS		=> $const_clstatus_created
#      };
#      (tied %clients)->shunlock;

      my $sql = "INSERT INTO clients VALUES (NULL, \"$cid\", \"$this_client{SYS}\", $this_client{SPED_SIZE}, $this_client{JOBC_SIZE}, $this_client{RESC_SIZE}, $this_client{DEXM_INTERNAL}, $this_client{DEXM_FTP}, $this_client{DEXM_COPY}, 1, \"$remip\", 0, \"\")";
      print "DEBUG: SQL command for adding a client is:\n       $sql\n";
      my $insert_session = $dbh->prepare($sql);
      $insert_session->execute;

      # --- some quick and dirty code for nfs: ---
      system("mkdir $dexm_nfs_path_in$cid_to_send");
      system("chmod 777 $dexm_nfs_path_in$cid_to_send");
      system("mkdir $dexm_nfs_path_out$cid_to_send");
      system("chmod 777 $dexm_nfs_path_out$cid_to_send");
    }
    else
    {
      $ret = "141 'LAST_FP' IS NOT ALLOWED HERE.\n142 YOU COULD NEED 'HELP'.";
    }
  }

  # ---------- GET (get a job)  -----------------------------------------
  elsif($command =~ /^GET\s*(.*)/)
  {
    my $get_command = $1;
    if($get_command eq "JOB")
    {
      if($allowed_commands{GET} == 1)
      {
        my $job_found = 0;

        my $sth = $dbh->prepare("SELECT status FROM clients WHERE cid='$cid'");
        my $ok = $sth->execute;
        my $job_name_exists = ((my ($client_status) = $sth->fetchrow_array));
        $sth->finish;

        if($client_status == $const_clstatus_rendering)
        {
          $ret = "290 DENIED! YOU ARE STILL RENDERING.";
          $msg = "You can't get a new job, because you are still rendering and I may not give you two jobs.";
        }
        else
        {
          my $job_name = "";
          if(&count_projects>0)
          {
            foreach $project_name (keys %projects)
            {
              if($projects{$project_name}->{OPEN_JOBS})
              { 
                my @open_jobs = @{$projects{$project_name}->{TO_RENDER}};
                my $this_job = pop(@open_jobs);
                if($this_job ne "")
                {  
                  my $this_job_rendering = $this_job;
                  $this_job_rendering =~ s/\.inc/\.tga/;
                  my @in_rendering = @{$projects{$project_name}->{IN_RENDERING}};
                  push(@in_rendering, $this_job_rendering);

                  (tied %projects)->shlock();
                  $projects{$project_name} = {
                    OPEN_JOBS		=> ($#open_jobs != -1),
                    FRAME_FILES		=> [ @{$projects{$project_name}->{FRAME_FILES}} ],
                    COPY_FILES		=> [ @{$projects{$project_name}->{COPY_FILES}} ],
                    FINISHED_FILES	=> [ @{$projects{$project_name}->{FINISHED_FILES}} ],
                    TO_RENDER		=> [ @open_jobs ],
                    IN_RENDERING	=> [ @in_rendering ],
                    IN_PATH		=> $projects{$project_name}->{IN_PATH}, 
                    OUT_PATH		=> $projects{$project_name}->{OUT_PATH},
                    RENDER_WIDTH	=> $projects{$project_name}->{RENDER_WIDTH},
                    RENDER_HEIGHT	=> $projects{$project_name}->{RENDER_HEIGHT}, 
                    RENDER_FPS		=> $projects{$project_name}->{RENDER_FPS},
                    RENDERER		=> $projects{$project_name}->{RENDERER},
                    RENDER_PARAMS	=> $projects{$project_name}->{RENDER_PARAMS},
                    RENDER_FILE		=> $projects{$project_name}->{RENDER_FILE},
                    RENDER_INCFILE	=> $projects{$project_name}->{RENDER_INCFILE}
                  };
                  (tied %projects)->shunlock();

                  $ret = "211 OK WILL GIVE YOU WORK:\n" .
                    "211 PROJECTNAME:     $project_name\n" .
                    "211 THIS FILE:       $this_job\n" .
                    "211 COPY FILES:      " . join(",",@{$projects{$project_name}->{COPY_FILES}},$this_job) . "\n" .
                    "211 WIDTH x HEIGHT:  $projects{$project_name}->{RENDER_WIDTH} x $projects{$project_name}->{RENDER_HEIGHT}\n" .
                    "211 RENDERER:        $projects{$project_name}->{RENDERER}\n" .
                    "211 RENDER PARAMS:   $projects{$project_name}->{RENDER_PARAMS}\n" .
                    "211 RENDER FILE:     $projects{$project_name}->{RENDER_FILE}\n" .
                    "211 RENDER INC FILE: $projects{$project_name}->{RENDER_INCFILE}\n" .
                    "212 HAVE A NICE RENDERING!";
                  $msg = "I have send you a job. Please render it and send it back.";
                  $job_found = 1;
                  };
                (tied %clients)->shunlock;
       	        last;
              }
            }
            (tied %projects)->shunlock();
          
            if(!$job_found)
            {
              $ret = "290 NO MORE OPEN JOBS, SORRY.";
              $msg = "There are no more jobs to render on this server.";
              $allowed_commands{GET} = 0;
            }
          }
          else
          {
            $ret = "290 THERE ARE NO OPEN PROJECTS ON THE SERVER, SORRY.";
            $msg = "All projects on the server are finished. Please retry later.";
            print "DEBUG: No projects -> no jobs (&count_projects)\n";
          }
        }
      }
      else
      {
        $ret = "141 'GET' IS NOT ALLOWED HERE.\n142 YOU COULD NEED 'HELP'.";
      }
    }
    elsif($get_command =~ /^FILE/)
    {
      my $local_cmdline = $orig_cmdline;
      $local_cmdline =~ s/^get\s*file\s*(\S*)\s*(\S*)$//i;
      my $project_name = $1;
      my $local_filename = $2;
#      print("DEBUG: (projectname:$project_name) cp $projects{$project_name}->{IN_PATH}$local_filename $dexm_nfs_path_in$cid/$local_filename\n");
      if($projects{$project_name} eq "")
      {
        $ret = "390 NO SUCH PROJECT!";
        $msg = "There is no project with that name on this server.";
      } 
      elsif($local_filename eq "")
      {
        $ret = "390 YOU SPECIFIED NO FILE TO COPY!";
        $msg = "SYNTAX for GET: get file <project_name> <file_name>";
      }
      elsif(-e "$projects{$project_name}->{IN_PATH}$local_filename")
      { 
        system("cp $projects{$project_name}->{IN_PATH}$local_filename $dexm_nfs_path_in$cid/$local_filename");
        $ret = "310 FILE $local_filename COPIED IN YOUR IN-PATH";
        $msg = "";
      }
      else
      {
        $ret = "390 FILE NOT FOUND! PLEASE RETRY...";
        $msg = "The requested file was not locally on the server. Perhaps another file?";
      }     
    }
    elsif($get_command =~ /^HELP\s*(.*)/)
    {
      my $help_request = $1;
      $ret = "720 NO HELP IMPLEMENTED YET!";
      $msg = "";
    }
    else
    {
      $ret = "230 MAL FORMED GET-COMMAND. TRY 'HELP GET'!";
      $msg = "Your GET command was formed incorrectly. More info with 'HELP GET'.";
    }
  }

  # ---------- SEND RESULTS --------------------------------------------------
  elsif($command =~ /^SEND/)
  {
    if($allowed_commands{SEND} == 1)
    {
      my $send_command = $orig_cmdline;
      $send_command =~ s/^send\s*//i;
      my ($project_name, $file_name) = split(/ /, $send_command);
      
      if($projects{$project_name} eq "")
      {
        $ret = "390 NO SUCH PROJECT!";
        $msg = "There is no project with that name on this server.";
      } 
      elsif($file_name eq "")
      {
        $ret = "390 YOU SPECIFIED NO FILE TO SEND HERE!";
        $msg = "SYNTAX for SEND: send <project_name> <file_name>";
      }
      elsif(-e "$dexm_nfs_path_out$cid/$file_name")
      { 
        print("DEBUG: cp $dexm_nfs_path_out$cid/$file_name $projects{$project_name}->{OUT_PATH}$file_name\n");
        system("cp $dexm_nfs_path_out$cid/$file_name $projects{$project_name}->{OUT_PATH}$file_name");
        $ret = "310 GOT FILE $file_name";
        $msg = "";
          
        # --- first we change the client information ---
        # REFERENCE TO NOTE [2] AT THE BEGINNING!!!
        (tied %clients)->shlock;
        $clients{$cid_to_send} = {
          SYS 		=> $clients{$cid}->{SYS}, 
          SPED_SIZE 	=> $clients{$cid}->{SPED_SIZE},
          SPED_UNIT 	=> $clients{$cid}->{SPED_UNIT},
          JOBC_SIZE 	=> $clients{$cid}->{JOBC_SIZE},
          JOBC_UNIT 	=> $clients{$cid}->{JOBC_UNIT},
          RESC_SIZE 	=> $clients{$cid}->{RESC_SIZE},
          RESC_UNIT 	=> $clients{$cid}->{RESC_UNIT},
          RR		=> [ @{$clients{$cid}->{RR}} ],
          FP		=> [ @{$clients{$cid}->{FP}} ],
          DEXM		=> $clients{$cid}->{DEXM},
          IP		=> $remip,
          PID		=> $$,
          CONNECTED	=> 1,
          STATUS	=> $const_clstatus_none
        };
        (tied %clients)->shunlock;

        # --- add this file to FINISHED_FILES ---
        my @finished_files = @{$projects{$project_name}->{FINISHED_FILES}};
        push(@finished_files, $file_name);

        # --- delete this file form array IN_RENDERING ---
        my @in_rendering_new = ();
        my $entry = "";
        foreach $entry (@{$projects{$project_name}->{IN_RENDERING}})
        {
          if($entry ne $file_name)
          {
            print "DEBUG: $entry != $file_name, adding it\n";
            push(@in_rendering_now, $entry);
          }
          else
          {
            print "DEBUG: $entry == $file_name, ignoring it\n";
          }
        }

        my @frame_files = @{$projects{$project_name}->{FRAME_FILES}};

        # --- print the new values to %projects ----
        (tied %projects)->shlock();
        $projects{$project_name} = {
          OPEN_JOBS		=> 1, 
          FRAME_FILES		=> [ @frame_files ],
          COPY_FILES		=> [ @{$projects{$project_name}->{COPY_FILES}} ],
          FINISHED_FILES	=> [ @finished_files ],
          TO_RENDER		=> [ @{$projects{$project_name}->{TO_RENDER}} ],
          IN_RENDERING		=> [ @in_rendering_now ],
          IN_PATH		=> $projects{$project_name}->{IN_PATH}, 
          OUT_PATH		=> $projects{$project_name}->{OUT_PATH},
          RENDER_WIDTH		=> $projects{$project_name}->{RENDER_WIDTH},
          RENDER_HEIGHT		=> $projects{$project_name}->{RENDER_HEIGHT}, 
          RENDER_FPS		=> $projects{$project_name}->{RENDER_FPS},
          RENDERER		=> $projects{$project_name}->{RENDERER},
          RENDER_PARAMS		=> $projects{$project_name}->{RENDER_PARAMS},
          RENDER_FILE		=> $projects{$project_name}->{RENDER_FILE},
          RENDER_INCFILE	=> $projects{$project_name}->{RENDER_INCFILE}
        };
        (tied %projects)->shunlock();

        print "DEBUG: frame finished, $#finished_files, $#frame_files\n";
        if($#finished_files == $#frame_files)
        {
          server_log("Project $project_name is finished!");
          (tied %projects)->shlock();
          $projects{$project_name} = {
            OPEN_JOBS		=> 0, 
            FRAME_FILES		=> [ @frame_files ],
            COPY_FILES		=> [ @{$projects{$project_name}->{COPY_FILES}} ],
            FINISHED_FILES	=> [ @{$projects{$project_name}->{FINISHED_FILES}}, $file_name ],
            TO_RENDER		=> [ ],
            IN_RENDERING	=> [ ],
            IN_PATH		=> $projects{$project_name}->{IN_PATH}, 
            OUT_PATH		=> $projects{$project_name}->{OUT_PATH},
            RENDER_WIDTH	=> $projects{$project_name}->{RENDER_WIDTH},
            RENDER_HEIGHT	=> $projects{$project_name}->{RENDER_HEIGHT}, 
            RENDER_FPS		=> $projects{$project_name}->{RENDER_FPS},
            RENDERER		=> $projects{$project_name}->{RENDERER},
            RENDER_PARAMS	=> $projects{$project_name}->{RENDER_PARAMS},
            RENDER_FILE		=> $projects{$project_name}->{RENDER_FILE},
            RENDER_INCFILE	=> $projects{$project_name}->{RENDER_INCFILE}
          };
          (tied %projects)->shunlock();
          print "DEBUG: Project $project_name is finished!\n";
        }

      }
      else
      {
        $ret = "390 FILE NOT FOUND! PLEASE RETRY...";
        $msg = "The send file did not arrive on the server.";
      }
      $allowed_commands{ADD_FP} = 0;
      $allowed_commands{LAST_FP} = 0;
      $allowed_commands{DEXM} = 1;
    }
    else
    {
      $ret = "141 'SEND' IS NOT ALLOWED HERE.\n142 YOU COULD NEED 'HELP'.";
    }
  }

  # ---------- PROJECT  -----------------------------------------
  elsif($command =~ /^PROJECT/)
  {
    my $project_commands = $orig_cmdline;
    $project_commands =~ s/^project\s*//i;

    my ($project_cmd, $project_param) = split(/ /,$project_commands);
    
    if(uc($project_cmd) eq "ADD")
    {
      server_log("Adding project from file $project_param\n");
      $msg = &read_rif($project_param);
      if($msg =~ /^OK/)
      {
        $ret = "110 OK, FILE $project_param WAS ADDED AS PROJECT";
      }
      else
      {
        $ret = "191 FAILED ADDING $project_param AS PROJECT\n192 REASON: $msg";
      }
      my $cprj = &count_projects;
      print "DEBUG: Actual count of projects: $cprj\n"; 
    }
    elsif(uc($project_cmd) eq "DELETE")
    {
    }
    elsif(uc($project_cmd) eq "FREEZE")
    {
    }
    elsif(uc($project_cmd) eq "UNFREEZE")
    {
    }
    elsif(uc($project_cmd) eq "INFO")
    {
    }
    else
    {
      $ret = "190 MAL FORMED 'PROJECT' COMMAND";
      $msg = "PROJECT SYNTAX: project (add|delete|freeze|unfreeze|info) (file name|project name)";
    }
  }

  # ---------- DEBUG ---------------------------------------------------------
  elsif($command =~ /^DEBUG\s*(.*)$/)
  {
    my $debug_param = $1;
    if($debug_param =~ /^PASSWD\s*(.*)$/)
    {
      $passwd = $1;
      if($passwd eq uc("AniTMT"))
      { 
        $allow_debug = 1;
        $ret = "881 PASSWORD ACCEPTED\n882 YOU CAN DEBUG NOW, 'DEBUG HELP' FOR HELP";
        $msg = "You enabled debug mode. Send 'debug help' for more help.";
      }
      else
      {
        $allow_debug = 0;
        $ret = "890 ACCESS DENIED! PASSWORD INCORRECT ($passwd)";
        $msg = "You failed enabling debug mode. Password was incorrect!";
      }
    }
    else
    {
      my $command = $debug_param;
      if($allow_debug)
      {
        if($command eq "OFF")
        {
          $allow_debug = 0;
          $ret = "810 DEBUG MODE DISABLED.";
          $msg = "You disabled debug mode. To use it again, please login again.";
        }
        elsif($command eq "INFO")
        {
          $ret = "811 SERVER INFORMATION:\n" .
#                 "811 server ip: $server_ip\n" .
                 "811 server ip: - unkown -\n" .
                 "811 server port: $server_port\n" .
                 "812 DEBUG END";
          $msg = "";
        }
        elsif($command eq "PIDS")
        {
          my $key = "none";
          $ret = "811 SEVER PID INFORMATION:\n" .
                 "811 parent pid:           $parent_pid\n" .
                 "811 client pids:\n" .
                 "811   pid:  ip:        last command:\n" .
                 "811   --- not implemented yet --- (SQL change)\n";
#          (tied %client_pids)->shlock();
#          foreach $key (keys %client_pids)
#          {
#            $ret = $ret . "811   $key  $client_pids{$key}->{IP}  $client_pids{$key}->{LAST}\n";
#          }        
#          (tied %client_pids)->shunlock();
          $ret = $ret . "811 this connections pid: $$";
          $msg = "";
        }
        elsif($command eq "RIF")
        {
          $ret = "811 RIF INFORMATION:\n";
          foreach $key (keys %rif)
          {
             $ret = $ret . "811 $key => $rif{$key}\n";
          }
          $ret = $ret . "812 END OF RIF INFORMATION";
          $msg = "";
        } 
        elsif($command eq "PROJECTS")
        {
          my @dummy_array = ();
          $ret = "811 INFORMATION ABOUT THE PROJECTS:\n";
          $msg = "";
          foreach $key (keys %projects)
          {
            $ret = $ret . "811 PROJECT NAME: $key\n" .
                          "811 Open jobs?    ";
            if($projects{$key}->{OPEN_JOBS})
            {
              $ret = $ret . "yes\n";
            }
            else
            {
              $ret = $ret . "no\n";
            }
            @dummy_array = @{$projects{$key}->{COPY_FILES}};
            my $num_copy_files = $#dummy_array + 1;
            if($#dummy_array != -1)
            {
              $tmp_str = "811 FILES TO COPY:     " . join(", ",@{$projects{$key}->{COPY_FILES}}) . "\n";
              $ret = $ret . $tmp_str;
            }
            else
            {
              $ret = $ret . "811 FILES TO COPY:     -none-\n";
            }
            @dummy_array = @{$projects{$key}->{FRAME_FILES}};
            my $num_frame_files = $#dummy_array + 1;
            if($#dummy_array != -1)
            {
              $tmp_str = "811 FRAME FILES:   " . join(", ",@{$projects{$key}->{FRAME_FILES}}) . "\n";
#              $tmp_str = s/\.inc//;
              $ret = $ret . $tmp_str;
            }
            else
            {
              $ret = $ret . "811 FRAME FILES:   -none-\n";
            }
            @dummy_array = @{$projects{$key}->{FINISHED_FILES}};
            my $num_finished_files = $#dummy_array + 1;
            if($#dummy_array != -1)
            {
              $tmp_str = "811 FINISHED FILES:    " . join(", ",@{$projects{$key}->{FINISHED_FILES}}) . "\n";
#              $tmp_str = s/\.tga//;
#              $ret = $ret . $tmp_str;
            }
            else
            {
              $ret = $ret . "811 FINISHED FILES:    -none-\n";
            }
            @dummy_array = @{$projects{$key}->{TO_RENDER}};
            my $num_to_render = $#dummy_array + 1;
            if($#dummy_array != -1)
            {
              $tmp_str = "811 FILES TO RENDER:   " . join(", ",@{$projects{$key}->{TO_RENDER}}) . "\n";
#              $tmp_str = s/\.inc//;
              $ret = $ret . $tmp_str;
            }
            else
            {
              $ret = $ret . "811 FILES TO RENDER:   -none-\n";
            }
            @dummy_array = @{$projects{$key}->{IN_RENDERING}};
            my $num_in_rendering = $#dummy_array + 1;
            if($#dummy_array != -1)
            {
              $tmp_str = "811 FILES IN RENDERING:" . join(", ",@{$projects{$key}->{IN_RENDERING}}) . "\n";
#              $tmp_str = s/\.tga//;
              $ret = $ret . $tmp_str;
            }
            else
            {
              $ret = $ret . "811 FILES IN RENDERING:-none-\n";
            }
            my $percent_finished = $num_finished_files / $num_frame_files * 100;
            my $percent_finished_str = sprintf("%.2f", $percent_finished);
            my $percent_to_render = $num_to_render / $num_frame_files * 100;
            my $percent_to_render_str = sprintf("%.2f", $percent_to_render);
            my $percent_in_rendering = $num_in_rendering / $num_frame_files * 100;
            my $percent_in_rendering_str = sprintf("%.2f", $percent_in_rendering);
            $ret = $ret .
              "811 INPUT PATH:        $projects{$key}->{IN_PATH}\n" .
              "811 OUTPUT PATH:       $projects{$key}->{OUT_PATH}\n" .
              "811 RENDER SIZE:       $projects{$key}->{RENDER_WIDTH} x $projects{$key}->{RENDER_HEIGHT} (w x h)\n" .
              "811 RENDER FPS:        $projects{$key}->{RENDER_FPS}\n" .
              "811 RENDERER:          $projects{$key}->{RENDERER}\n" .
              "811 RENDER PARAMETER:  $projects{$key}->{RENDER_PARAMS}\n" .
              "811 RENDER FILE:       $projects{$key}->{RENDER_FILE}\n" .
              "811 RENDER INCLUDE IN: $projects{$key}->{RENDER_INCFILE}\n" .
              "811 STATISTICS:        files to copy:   $num_copy_files \n" .
              "811                    total frames:    $num_frame_files (100\%) \n" .
              "811                    still to render: $num_to_render ($percent_to_render_str\%) \n" .
              "811                    in rendering:    $num_in_rendering ($percent_in_rendering_str\%) \n" .
              "811                    finshed frames:  $num_finished_files ($percent_finished_str\%) \n";
          }
          $ret = $ret . "812 END";
        }
        elsif($command eq "CLIENTS DETAILED")
        {
          $ret = "811 DETAILED CLIENT INFORMATION\n";
          $msg = "";

          (tied %clients)->shlock();
          foreach $key (keys %clients)
          {
            $ret = $ret .
              "811 CID:             $key\n" .
              "811 IP:              $clients{$key}->{IP}\n" .
              "811 SYS:             $clients{$key}->{SYS}\n" .
              "811 SPED:            $clients{$key}->{SPED_SIZE} $clients{$key}->{SPED_UNIT}\n" .
              "811 JOBC:            $clients{$key}->{JOBC_SIZE} $clients{$key}->{JOBC_UNIT}\n" .
              "811 RESC:            $clients{$key}->{RESC_SIZE} $clients{$key}->{RESC_UNIT}\n" .
              "811 RR:              " . join(" ", @{$clients{$key}->{RR}}) . "\n" .
              "811 FP:              " . join(" ", @{$clients{$key}->{FP}}) . "\n" .
              "811 DEXM             $clients{$key}->{DEXM}\n" .
              "811 Connection?      $clients{$key}->{CONNECTED} = ";
            if($clients{$key}->{CONNECTED})
            {
              $ret = $ret . "connected\n";
            }
            else
            {
              $ret = $ret . "not connected\n";
            }
            $ret = $ret . "811 Client status:   $clients{$key}->{STATUS} = ";
            if($clients{$key}->{STATUS} == $const_clstatus_none)
            {
              $ret = $ret . "none\n";
            }
            elsif($clients{$key}->{STATUS} == $const_clstatus_talking)
            {
              $ret = $ret . "talking\n";
            } 
            elsif($clients{$key}->{STATUS} == $const_clstatus_created)
            {
              $ret = $ret . "created\n";
            } 
            elsif($clients{$key}->{STATUS} == $const_clstatus_rendering)
            {
              $ret = $ret . "rendering\n";
            } 
            elsif($clients{$key}->{STATUS} == $const_clstatus_get_data)
            {
              $ret = $ret . "get data\n";
            } 
            elsif($clients{$key}->{STATUS} == $const_clstatus_send_data)
            {
              $ret = $ret . "send data\n";
            } 
            else
            {
              $ret = $ret . "ERROR: unknown!\n";
            }
          }
          (tied %clients)->shunlock();
          $ret = $ret . "812 END OF LIST";
        }
        elsif($command eq "HELP")
        {
          $ret = $ret_debug_help;
          $msg = "";
        }
        else
        {
          $ret = "830 MAL FORMED DEBUG REQUEST";
          $msg = "The debug command was mal formed. Try 'debug help'.";
        }
      }
      else
      {
        $ret = "890 DEBUG NOT ALLOWED! USE 'DEBUG PASSWD <secret>' TO ENABLE.";
        $msg = "You tried to enter debug mode, but debug is not enabled! Use 'debug passwd <secret>' to enable it.";
      }
    }
  }

  # ---------- HELP ----------------------------------------------------------
  elsif($command eq "HELP")
  {
    $ret = "801 SORRY, 'HELP' IS NOT YET IMPLEMENTED!\n802 VISIT http://www.theofel.de/anitmt/ FOR DOCUMENTATION AND NEW VERSIONS";
  }
  
  # ---------- MSG -----------------------------------------------------------
  elsif($command eq "MSG")
  {
    $ret = "800 $msg"; 
  }

  # ---------- UNKNOWN COMMAND -----------------------------------------------
  else
  {
    $ret = "121 UNKNOWN COMMAND!\n122 YOU COULD NEED 'HELP'."; 
  }

  return($ret);
} # --------------------------------------------------------------------------
    
sub return_error
# ----------------------------------------------------------------------------
# task:		sends an error msg to the client via $SOCKET
# last changed: 2000-01-02
# ----------------------------------------------------------------------------
{
  my ($SOCKET, @message) = @_;
  print $SOCKET @message;
}

# ---------- sub for handlers ------------------------------------------------
sub child_handler
{
  wait;
}

sub exit_request_handler
# ----------------------------------------------------------------------------
# task:		shuts down the server and writes cid.backup
# author:	Jan Theofel, jan@theofel.de
# last changed: 2001-07-01
# ----------------------------------------------------------------------------
{
  my ($recvsig) = @_;
  $SIG{'INT'} = $SIG{'QUIT'} = 'IGNORE';
  close(SERVERSOCKET);
  close(CHILDSOCKET);
  if($parent_pid == $$)
  { 
    server_log("Quitting parent process with pid $$ on signal $recvsig\n");
#   THE FOLLOWING SAVING OF THE CLIENTS IS NOT NEEDED ANY MORE WITH SQL
#    server_log("Storing client information in $client_info_filename\n");
#    if(open(BACKUP,">$client_info_filename"))
#    {
#      print BACKUP "DO NOT EDIT OR MODIFY THIS FILE! IT IS AUTOMATICALLY GENERATED!\n";
#      print BACKUP "Visit: http://www.theofel.de/anitmt/ for more information about it!\n\n";
#
#      (tied %clients)->shlock();
#      foreach $key (keys %clients) 
#      {
#        print BACKUP "CID=$key\n";
#        print BACKUP "SYS=$clients{$key}->{SYS}\n"; 
#        print BACKUP "SPED_SIZE=$clients{$key}->{SPED_SIZE}\n";
#        print BACKUP "SPED_UNIT=$clients{$key}->{SPED_UNIT}\n";
#        print BACKUP "JOBC_SIZE=$clients{$key}->{JOBC_SIZE}\n";
#        print BACKUP "JOBC_UNIT=$clients{$key}->{JOBC_UNIT}\n";
#        print BACKUP "RESC_SIZE=$clients{$key}->{RESC_SIZE}\n";
#        print BACKUP "RESC_UNIT=$clients{$key}->{RESC_UNIT}\n";
#        print BACKUP "RR=" . join(",", @{$clients{$key}->{RR}}) . "\n";
#        print BACKUP "FP=" . join(",", @{$clients{$key}->{FP}}) . "\n";
#        print BACKUP "DEXM=$clients{$key}->{DEXM}\n";
#        print BACKUP "IP=$clients{$key}->{IP}\n";
#        print BACKUP "PID=$clients{$key}->{PID}\n";
#        print BACKUP "CONNECTED=$clients{$key}->{CONNECTED}\n";
#        print BACKUP "STATUS=$clients{$key}->{STATUS}\n";
#        print BACKUP "\n"; 
#        delete($clients{$key});
#      }
#      # no unlocking any more, we quit!!!
#
#      (tied %client_pids)->shlock();
#      foreach $key (keys %client_pids)
#      {
#        delete($client_pids{$key});
#      }
#
#      (tied %projects)->shlock();
#      foreach $key (keys %projects)
#      {
#        delete($projects{$key});
#      }
#
#      close(BACKUP);
#    }
#    else
#    {
#      server_log("... FAILED TO OPEN FILE!");
#      (tied %clients)->shlock();
#      foreach $key (keys %clients) 
#      {
#        delete($clients{$key});
#      }
#      # no unlocking any more, we quit!!!
#    }
  }
  else
  {
    server_log("Quitting child process with pid $$ on signal $recvsig\n");
  }
  $dbh->disconnect;
  exit(1);
}

sub signal_sighup
{
  print "Revieved signal SIGHUP, restarting...\n";
}

sub time_as_string
# ----------------------------------------------------------------------------
# task:         returns the current date & time as string 
# last changed: 2000-01-09
# ----------------------------------------------------------------------------
{
  $ret = "";
  $ret = "2000-01-09";
  return($ret);
}

sub pro_log
# ----------------------------------------------------------------------------
# task:         logs into the protocoll log file $protocoll_log 
# last changed: 2000-01-09
# ----------------------------------------------------------------------------
{
  $from_to = $_[0];
  $logstring = "--- \@ " . time_as_string . " --- $from_to\n" . $_[1];
  open(LOG,">>$protocoll_log"); # || error_log("Can't open protocoll log file $protocoll_log!\n");
  print LOG $logstring;
  close(LOG); 
}  

sub server_log
# ----------------------------------------------------------------------------
# task:         logs into the server log file $server_log
# last changed: 2000-01-09
# ----------------------------------------------------------------------------
{
  $logstring = time_as_string . " - " . $_[0];
  open(LOG,">>$server_log") || error_log("Can't open server log file $server_log!\n");
  print LOG $logstring;
  close(LOG);
}

sub error_log
# ----------------------------------------------------------------------------
# task:         logs into the error log file $error_log
# last changed: 2000-01-09
# ----------------------------------------------------------------------------
{
  $logstring = time_as_string . " - " . $_[0];
  open(LOG,">>$error_log");
  print LOG $logstring;
  print $logstring;
  close(LOG);
}

sub check_params
# ----------------------------------------------------------------------------
# task:         parses and executes the command line params 
# author:	Jan Theofel, jan@theofel.de
# last changed: 2000-02-26
# ----------------------------------------------------------------------------
{

  print "DEBUG: Parsing command line options\n";
  server_log("The command line options are parsed...\n");
  GetOptions(
    "clear-logs"	=>	\$opt_clear_logs,
    "help"		=>	\$opt_help,
    "version"		=>	\$opt_version,
    "server-log=s"	=>	\$server_log,
    "protocoll-log=s"	=>	\$protocoll_log,
    "error-log=s"	=>	\$error_log,
    "server-port=i"	=>	\$server_port,
    "rif=s"		=>	\$opt_riffile); 

  if($opt_help) { &print_usage; }
  if($opt_version) { &print_version; }
  if($opt_clear_logs) { &do_clear_logs; }  
  
  if(defined($opt_riffile))
  {
    server_log("Starting with project from file $opt_riffile...\n");
    my $msg = &read_rif($opt_riffile);
    if($msg =~ /^OK/)
    {
      server_log("RIF read succesfully: $msg\n");
    }
    else
    {
      server_log("Reading RIF failed: $msg\n");
    }
  }
  else
  {
    server_log("Starting without a project. Waiting for incoming projects...\n"); 
  }
     
}

sub print_usage
# ----------------------------------------------------------------------------
# task:		prints the usage to the screen and exits
# last-changed:	2000-01-23
# ----------------------------------------------------------------------------
{
  print <<END_OF_USAGE;

  anitmt-server  --  the NRP server for the anitmt program bundle
  
  This program is published under the terms of the compGPL license.
  It comes with absolutly NO WARRENTY. USE IT AT YOUR OWN RSIK!
  (For more details read the file 'license'.)
  
  Usage:
    --clear-logs                clears all logfiles
    --server-log filename       set the name of the server log file
    --protocoll-log filename    set the name of the protocoll log file
    --error-log filename        set the name of the error log file
    --server-port port          set the port the server will listen to
    --rif riffile		access the riffile directly
    --help                      this little help text
    --version                   displays the program name and version

  Further informations can be found at:    http://www.theofel.de/anitmt/
  Contact to the authors is:               jan\@theofel.de
END_OF_USAGE

  die("\n");
}

sub print_version
# ----------------------------------------------------------------------------
# task:		prints the version to the screen and exits
# last-changed:	2000-01-23
# ----------------------------------------------------------------------------
{
  print("\n");
  print("anitmt-server  --  the NRP server for the anitmt program bundle\n");
  print("  server version:    $server_version (build $server_build)\n");
  print("  protocoll version: $protocoll_version\n");
  die("\n");
}

sub do_clear_logs 
# ----------------------------------------------------------------------------
# task:         clears all the log files 
# last-changed: 2000-01-23
# ----------------------------------------------------------------------------
{
  
  open(ERROR_LOG,">$error_log") || die("ERROR: Can't open $error_log for clearing!");
  close(ERROR_LOG);

  open(SERVER_LOG,">$server_log") || die("ERROR: Can't open $server_log for clearing!");
  close(SERVER_LOG);

  open(PROTOCOLL_LOG,">$protocoll_log") || die("ERROR: Can't open $protocoll_log for clearing!");
  close(PROTOCOLL_LOG);

  die("OK: log files were cleared\n");

}
  
sub read_rif
# ----------------------------------------------------------------------------
# task:		reads and executes a rif file (render information file)       
# author:	Jan Theofel, jan@theofel.de
# last-changed: 2001-12-02
# ----------------------------------------------------------------------------
{

  my $rif_filename = $_[0];
  my $ani_source_path = "";

  if(open(RIF,"<$rif_filename"))
  {
    while(<RIF>)
    {
      if($_ =~ /^\s*$/ || $_ =~ /^#.*$/) { next; }
      $_ =~ /^\s*(\S*)\s*=\s*\"{0,1}([^\"\s]*)\"{0,1}$/;
      $name = uc($1);
      $value = $2;
      $rif{$name}=$value;
    }
    close(RIF);

    if($rif_filename =~ /\//)
    {
      # split filename from the end:
      $rif_filename =~ /(.*\/)[^\/]*/;
      $ani_source_path = $1 . $rif{ANI_DIR};
    }
    else
    {
      $ani_source_path = "./";
    }

    print "DEBUG: Parsed \$ani_source_path is $ani_source_path\n";

    my $job_name = $rif{NAME};
    delete($rif{NAME});
    print "DEBUG: Parsed \$job_name is $job_name\n";

    my $sth = $dbh->prepare("SELECT author, comment FROM projects WHERE name='$job_name'");
    my $ok = $sth->execute;
    my $job_name_exists = ((my ($existing_author, $existing_comment) = $sth->fetchrow_array));
    $sth->finish;

    if($job_name_exists)
    {
      return("Error: Jobname already in use (author: $existing_author, comment: $existing_comment)");
    }

    # --- add all files in that dir in @local_ani_files ---
    my @local_ani_files = ();
    my @local_ani_frames = ();
    my @local_ani_copies = ();
    my $filename = "";
    my $regex = $rif{FRAME_SRC};
    $regex =~ /(.*)\%(\d*)d(.*)/;
    my $befor = $1; 
    my $digits = $2;
    my $after = $3;
    $regex = $befor . "\\d{$digits}" . $after;
    print "DEBUG: final regex for filedetection: $regex\n";
  
    opendir(ANI,$ani_source_path) || die "FATAL ERROR: Can't open ani dir!";
    while(defined($filename = readdir(ANI)))
    {
      if($filename ne "." && $filename ne "..")
      {
        push(@local_ani_files, $filename);
      }
    }
    closedir(ANI);

    foreach $filename (@local_ani_files)
    {
      if($filename =~ /$regex/)
      {
        push(@local_ani_frames, $filename);
      }
      else
      {
        push(@local_ani_copies, $filename);
      }
    }
  
    print "DEBUG: files with frames:   " . join(",",@local_ani_frames) . "\n";
    print "DEBUG: files to copy:       " . join(",",@local_ani_copies) . "\n";

    if (!-e "$default_out_path$job_name")
    { 
      print("DEBUG: syscall [mkdir $default_out_path$job_name]\n");
      if(system("mkdir $default_out_path$job_name") != 0)
      {
        return("Error: Can't create path $default_out_path$job_name ($!)");
      }
      
      print("DEBUG: syscall [chmod 777 $default_out_path$job_name]\n");
      if(system("chmod 777 $default_out_path$job_name") != 0)
      {
        return("Error: Can't execute CHMOD on path $default_out_path$job_name ($!)");
      }
#      @files_to_render = sort {$b cmp $a} @local_ani_frames;
    }
    else
    { 
      return("Error: Output path for job already exists!");
    }

    # --- insert the project in the SQL database ---
    my $sql = "INSERT INTO projects VALUES (NULL, \"$job_name\", \"author unknown\", \"no comment given\", \"$ani_source_path\", \"$default_out_path$job_name/\", $rif{WIDTH}, $rif{HEIGHT}, \"$rif{MAIN_FILE}\", \"$rif{FRAME_SRC}\", \"$rif{FRAME_DEST}\", $rif{FPS}, $rif{FRAMES}, 0)"; 
    server_log("DEBUG: SQL command for adding the project is:\n  $sql\n");
    my $insert_session = $dbh->prepare($sql);
    $insert_session->execute;

    $project_id = $insert_session->{'mysql_insertid'};

    # --- insert the frames in the SQL database ---
    foreach $filename (sort @local_ani_frames)
    {
      my $sql = "INSERT INTO frames VALUES (NULL, $project_id, \"$rif{RAYTRACER}\", \"$rif{PARAMS}\", 0, \"$filename\", \"not rendered\", 1)";
      server_log("DEBUG: SQL command for adding the frame is:\n  $sql\n");
      my $insert_session = $dbh->prepare($sql);
      $insert_session->execute;
    }

    # --- insert the copy-files in the SQL database ---
    foreach $filename (sort @local_ani_copies)
    {
      my $sql = "INSERT INTO copy_files VALUES (NULL, $project_id, \"$filename\")";
      server_log("DEBUG: SQL command for adding the copy file is:\n  $sql\n");
      my $insert_session = $dbh->prepare($sql);
      $insert_session->execute;
    }

    # --- security check for number of frames ---
    if($#local_ani_files+1 != $rif{FRAMES})
    {
      server_log("WARNING: Number of files not equal with given frame number (project $job_name)\n");
    }

    return("OK: Added file $_[0] succesfully");
  }
  else
  {
    return("ERROR: File $_[0] not found to include as RIF!");
  }
}

sub count_projects
# ----------------------------------------------------------------------------
# task:		counts all projects
# author:	Jan Theofel, jan@theofel.de
# last-changed: 2001-07-01
# ----------------------------------------------------------------------------
{
  my $count_projects = 0;

  my $sth = $dbh->prepare("SELECT name, author, comment FROM projects");
  my $ok = $sth->execute;
  while ($sth->fetchrow_array)
  {
    $count_projects++;
  }
  $sth->finish;

  return $count_projects;
  
}

__END__                 # ---------- end of the anitmt-server script ---------
