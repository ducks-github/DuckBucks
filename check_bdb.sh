# Check Berkeley DB version
ls -l /usr/lib/libdb*.so
ls -l /usr/include/db*.h
# Specifically look for db_cxx.h
ls -l /usr/include/db_cxx.h

# Test BDB compilation
echo "#include <db_cxx.h>
int main() { return 0; }" > /tmp/test_db.cpp
g++ -o /tmp/test_db /tmp/test_db.cpp -ldb_cxx