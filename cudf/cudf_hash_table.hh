
/*******************************************************/
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010         */
/* Distributed under GPLv3                             */
/*******************************************************/


#ifndef CUDF_HASH_TABLE_H
#define CUDF_HASH_TABLE_H

#include <cudf/cudf.hh>

//#define HASHMAP 1

#ifdef HASHMAP
#include <ext/hash_map>
#else
#include <tr1/unordered_map>
#endif

#define HASH_STRING 1 

#ifdef HASH_STRING

#ifdef HASHMAP
typedef __gnu_cxx::hash_map<string, CUDFVirtualPackage *> an_hash_table;
typedef __gnu_cxx::hash_map<string, CUDFVirtualPackage *>::iterator an_hash_table_iterator;
#else
typedef tr1::unordered_map<string, CUDFVirtualPackage *> an_hash_table;
typedef tr1::unordered_map<string, CUDFVirtualPackage *>::iterator an_hash_table_iterator;
#endif

#else

struct eqstr {
  bool operator()(const char* s1, const char* s2) const {
    return (strcmp(s1, s2) == 0);
  }
};

#ifdef HASHMAP
typedef __gnu_cxx::hash_map<const char*, CUDFVirtualPackage *, __gnu_cxx::hash<const char*>, eqstr> an_hash_table;
typedef __gnu_cxx::hash_map<const char*, CUDFVirtualPackage *, __gnu_cxx::hash<const char*>, eqstr>::iterator an_hash_table_iterator;
#else
typedef tr1::unordered_map<const char*, CUDFVirtualPackage *, tr1::hash<const char*>, eqstr> an_hash_table;
typedef tr1::unordered_map<const char*, CUDFVirtualPackage *, tr1::hash<const char*>, eqstr>::iterator an_hash_table_iterator;
#endif

#endif

//extern an_hash_table cudf_packages;

extern CUDFVirtualPackage *get_virtual_package(const char *pkgname);

#endif
