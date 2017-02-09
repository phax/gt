#
# gtprf.pl
#
# Part of GT2
#
# Read the resource file(s) and generate HRC and RC file
#
# Revision history:
# v0.01 - Initial release
# v0.02 - Added support for macros
#

my (%macros);

## replace all macros in the passed string
sub repl_macros ()
{
  my ($s, $lineno) = @_;
  while ($s =~ /\$\$(\w+)/)
  {
    if (!(defined $macros{$1}))
    {
      # double define in line ...
      print "ResGen Error($lineno): already defined: $line";
      $errs++;
    }
    else
    {
      # replace macro
      $s =~ s/\$\$(\w+)/$macros{$1}/;
    }
  }
  return $s;
}

## returns 1 if already defined, 0 if new defined
sub define_string ()
{
  my ($lineno, $strid, $lang, $all) = @_;

  # already defined this string for this language?
  if (defined $str_rc{$strid}{$lang})
  {
    if (!$all)
    {
      # double define in line ...
      print "ResGen Error($lineno): already defined: $line";
      $errs++;
    }
    return 1;
  }
  else
  {
    # Key = langID + StringID
    $str_rc{$strid}{$lang} = &repl_macros ($2, $lineno);
    return 0;
  }
}

# main reading routine
sub read_file ()
{
  my ($filename) = @_;

  # read complete file
  print "ResGen reading file $filename\n";
  open A, $filename || die "Failed to open file $filename";
  my (@lines) = <A>;
  close A;

  # parse file
  my ($nLineNo, $nLineCount) = (0, scalar (@lines));
  while ($nLineNo < $nLineCount)
  {
    my $line = $lines[$nLineNo];
    if ($line =~ /^LANG\s+(\w+)/)
    {
      print "ResGen adding language $1\n";
      push (@langs, $1);
    }
    elsif ($line =~ /^MACRO\s+(\w+)\s+(.+)\s*$/)
    {
      my ($sName, $sMacro, $nRepl) = ($1, $2, 0);
      if (defined $macros{$sName})
      {
        print "ResGen Error($nLineNo): macro $sName is already defined\n";
        $errs++;
      }
      else
      {
        my $sFullMacro = "";
        do
        {
          $nRepl = 0;
          if ($sMacro =~ s/^\s*\$\$(\w+)(.+)$/\2/)
          {
            # check if macro exists
            my $sRefMacro = $1;
            if (!(defined $macros{$sRefMacro}))
            {
              print "ResGen Error($nLineNo): macro $sRefMacro referenced in macro $sName is unknown\n";
              $errs++;
            }
            else
            {
              $nRepl++;
              $sFullMacro .= $macros{$sRefMacro};
            }
          }
          elsif ($sMacro =~ s/^\s*\"([^\"]*)\"(.+)$/\2/)
          {
            $nRepl++;
            $sFullMacro .= $1;
          }
        } while ($nRepl > 0);

        if ($sFullMacro =~ /\$\$/)
        {
          print "ResGen Error($nLineNo): macros may not contain textual references to other macros\n";
          $errs++;
        }
        else
        {
          # save macro
          $macros{$sName} = $sFullMacro;
        }
      }
    }
    elsif ($line =~ /^STR\s+(\w+)/)
    {
      # new string
      $strid = $1;

      if (defined $str_hrc{$strid})
      {
        print "ResGen Error($nLineNo): ".$strid." is already defined in line ".$str_hrc{$strid}."\n";
        $errs++;

        # skip string
        for ($nLineNo++; $nLineNo < $nLineCount; $nLineNo++)
        {
          last if ($lines[$nLineNo] =~ /^\s*\n/);  # end of this string?
        }
      }
      else
      {
        $str_hrc{$strid} = $nLineNo;   # for HRC
        for ($nLineNo++; $nLineNo < $nLineCount; $nLineNo++)
        {
          $line = $lines[$nLineNo];
          last if ($line =~ /^\s*\n/);  # end of this string?
          if ($line =~ /^\s+(\w+)\s*=(".*");\s*(\/\/.+)?\n/)
          {
            if ($1 eq "all")
            {
              # do it for all languages
              my ($errors, $langs) = (0, 0);
              foreach $lang (@langs)
              {
                $errors += &define_string ($nLineNo, $strid, $lang, 1);
                $langs++;
              }
              if ($errors == $langs)
              {
                print "ResGen Error($nLineNo): all is already defined: $line";
                $errs++;
              }
            }
            else
            {
              # define in given language
              &define_string ($nLineNo, $strid, $1, 0);
            }
          }
          else
          {
            # error in line
            print "ResGen Error($nLineNo): syntax error: $line";
            $errs++;
          }
        }
      }
    }
    elsif ($line =~ /^INCLUDE\s+(\w+\.\w+)/)
    {
      # recursive read
      &read_file ($1);
    }
    elsif ($line !~ /^\/\// && $line !~ /^\s*\n/)
    {
      # ignoring line
      print "ResGen Error($nLineNo): unexpected line: $line";
      $errs++;
    }

    $nLineNo++;
  }
}

sub create_hrc ()
{
  # print HRC file
  open H, ">gt.hrc" || die;
  print H "// This file is generated! Do NOT edit.\n";
  print H "#ifndef _GT_HRC_\n";
  print H "#define _GT_RHC_\n";

  # Starting index
  $nIndex = 10000;

  foreach $strid (sort keys %str_hrc)
  {
    print H "#define $strid ".$nIndex++."\n";
  }
  print H "#endif\n";
  close H;
}

# print content in a given language
sub create_rc_lang ()
{
  my ($lang, $pri, $sub) = @_;
  $MAX_STRTAB = 256; # max number of entries in a string table
  my ($nIndex, $lang_len) = (0, length ($lang));
  foreach $strid (sort keys %str_rc)
  {
    if (defined $str_rc{$strid}{$lang})
    {
      if ($nIndex == 0)
      {
        print R "LANGUAGE $pri,$sub\n";
        print R "STRINGTABLE {\n";
      }
      if ($nIndex++ == $MAX_STRTAB)
      {
        print R "}\n";
        print R "LANGUAGE $pri,$sub\n";
        print R "STRINGTABLE {\n";
        $nIndex = 1;
      }
      print R '  '.$strid.', '.$str_rc{$strid}{$lang}."\n";
    }
  }
  print R "}\n" if ($nIndex > 0);
}

sub create_rc ()
{
  open R, ">gtres.rc" || die;
  print R "// this file is generated! Do NOT edit.\n";
  print R "#include \"gt.hrc\"\n";
  print R "#include <windows.h>\n";

  &create_rc_lang ('en', 9, 2);
  #&create_rc_lang ('de', 7, 1);

  print R "#define GT_FILENAME     \"gtbasic\"\n";
  print R "#define GT_DESCRIPTION  \"General purpose library.\"\n";
  print R "#include \"gt_version.rc\"\n";
  close R;
}

## main routine :)
sub main ()
{
  print "ResGen Start\n";

  @langs = "";
  %str_hrc = ();
  %str_rc = ();
  $errs = 0;

  # start with file "gt.prf"
  &read_file ("gt.prf");
  if ($errs > 0)
  {
    exit (1);
  }

  &create_hrc ();
  &create_rc ();

  print "ResGen Done\n";
}

# entry point
&main ();
