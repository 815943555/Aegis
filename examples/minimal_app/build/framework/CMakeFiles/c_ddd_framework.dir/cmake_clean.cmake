file(REMOVE_RECURSE
  "libc_ddd_framework.a"
  "libc_ddd_framework.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang C)
  include(CMakeFiles/c_ddd_framework.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
