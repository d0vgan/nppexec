{
  s = $0
  gsub("\\\\", "\\\\", s) # why I have to double \\ in the first param???
  gsub("\"", "\\\"", s)
  gsub("\'", "\\\'", s)
  print "  _T(\"" s "\") _T_RE_EOL \\"
}
