BEGIN {
   OPT_ID_FMT   = "OPTS_USERMENU_ITEM%02d"
   OPT_FLAGS    = "OPTT_STR | OPTF_READWRITE"
   INI_SECTION  = "INI_SECTION_USERMENU"
   OPT_NAME_FMT = "%02d"
   OPT_INTVALUE = "0"
   OPT_STRVALUE = "NULL"

   OPTS_COUNT   = 60
   OPTS_1ST     = 40

   for ( i = OPTS_1ST; i < OPTS_1ST + OPTS_COUNT; i++ )
   {
       id = sprintf(OPT_ID_FMT, i+1)
       name = sprintf(OPT_NAME_FMT, i)
       flags = OPT_FLAGS
       sec = INI_SECTION
       ival = OPT_INTVALUE
       sval = OPT_STRVALUE
       
       print "    { " id ", " flags ","
       print "      " sec ", _T(\"" name "\"), " ival ", " sval " },"
   }
}

