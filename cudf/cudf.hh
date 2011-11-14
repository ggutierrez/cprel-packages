
/*******************************************************/
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010         */
/* Distributed under GPLv3                             */
/*******************************************************/


#ifndef _CUDF_H
#define _CUDF_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>

using namespace std;

class CUDFPropertyValue;

typedef vector<CUDFPropertyValue *> CUDFPropertyValueList;
typedef vector<CUDFPropertyValue *>::iterator CUDFPropertyValueListIterator;

// Type for the version of a package
typedef unsigned long long CUDFVersion;


class CUDFPackage;
class CUDFVersionedPackage;
class CUDFVirtualPackage;
class CUDFPackage_comparator;

// The different types of operations which can be done on a package
enum CUDFPackageOp { op_none, op_eq, op_neq, op_inf, op_sup, op_infeq, op_supeq};

class CUDFVpkg {
 public:
  CUDFVirtualPackage *virtual_package;
  CUDFPackageOp op;
  CUDFVersion version;
  
  CUDFVpkg(CUDFVirtualPackage *the_virtual_package, CUDFPackageOp the_op, CUDFVersion the_version) { 
    virtual_package = the_virtual_package;
    op = the_op;
    version = the_version;
  };
};

class CUDFPackage {
 public:
  char *name;
  int rank;
  char *versioned_name;
};

typedef vector<CUDFVpkg *> CUDFVpkgList;
typedef CUDFVpkgList::iterator CUDFVpkgListIterator;
typedef vector< CUDFVpkgList *> CUDFVpkgFormula;
typedef CUDFVpkgFormula::iterator CUDFVpkgFormulaIterator;

// The different types of keep operations
enum CUDFKeepOp { keep_none, keep_feature, keep_package, keep_version};

class CUDFVersionedPackage: public CUDFPackage {
public:

  CUDFVersion version;
  CUDFVpkgFormula *depends;
  CUDFVpkgList *conflicts;
  CUDFVpkgList *provides;
  bool installed;
  bool wasinstalled;
  CUDFKeepOp keep;
  CUDFPropertyValueList properties;

  CUDFVirtualPackage *virtual_package;

  CUDFVersionedPackage(const char *pkg_name, int my_rank);

  void set_version(CUDFVersion pkg_version);
};

class CUDFPackage_comparator {
public:
  bool operator()(CUDFVersionedPackage *p1, CUDFVersionedPackage *p2) {
    if (p1->version < p2->version)
      return true;
    else
      return false;
  }
};

typedef set<CUDFVersionedPackage *, CUDFPackage_comparator> CUDFVersionedPackageSet;
typedef CUDFVersionedPackageSet::iterator CUDFVersionedPackageSetIterator;

typedef vector<CUDFVersionedPackage *> CUDFProviderList;
typedef CUDFProviderList::iterator CUDFProviderListIterator;
typedef map<CUDFVersion, CUDFProviderList> CUDFVersionedProviderList;
typedef CUDFVersionedProviderList::iterator CUDFVersionedProviderListIterator;

class CUDFVirtualPackage: public CUDFPackage {
public:

  CUDFVersionedPackageSet all_versions;
  CUDFVersionedPackage *highest_installed;
  CUDFVersion highest_version;

  CUDFProviderList providers;
  CUDFVersionedProviderList versioned_providers;
  CUDFVersion highest_installed_provider_version;

  CUDFVirtualPackage(const char *pkg_name, int my_rank);   
};


class CUDFproblem {
public:
  CUDFVpkgList *install;
  CUDFVpkgList *remove;
  CUDFVpkgList *upgrade;

  CUDFproblem() {
    install = (CUDFVpkgList *)NULL;
    remove = (CUDFVpkgList *)NULL;
    upgrade = (CUDFVpkgList *)NULL;
  }
};


typedef vector<CUDFVersionedPackage *> CUDFVersionedPackageList;
typedef CUDFVersionedPackageList::iterator CUDFVersionedPackageListIterator;
typedef vector<CUDFVirtualPackage *> CUDFVirtualPackageList;
typedef CUDFVirtualPackageList::iterator CUDFVirtualPackageListIterator;

// Enums

typedef vector<char *> CUDFEnums;
typedef vector<char *>::iterator CUDFEnumsIterator;

extern char *get_enum(CUDFEnums *e, char *estr);

// Property types

class CUDFProperty;

typedef map<string, CUDFProperty *> CUDFProperties;
typedef CUDFProperties::iterator  CUDFPropertiesIterator;
extern CUDFProperties properties;

// Types allowed for properties
enum CUDFPropertyType { pt_none,
                        pt_bool, pt_int, pt_nat, pt_posint, pt_enum, pt_string, 
			pt_vpkg, pt_veqpkg, pt_vpkglist, pt_veqpkglist, pt_vpkgformula};

// Property values

class CUDFPropertyValue {
 public:
  CUDFProperty *property;
  int intval;
  char *strval;
  CUDFVpkg *vpkg;
  CUDFVpkgList *vpkglist;
  CUDFVpkgFormula *vpkgformula;

  CUDFPropertyValue(CUDFProperty *the_property, int the_value);
  CUDFPropertyValue(CUDFProperty *the_property, char *the_value);
  CUDFPropertyValue(CUDFProperty *the_property, CUDFVpkg *the_value);
  CUDFPropertyValue(CUDFProperty *the_property, CUDFVpkgList *the_value);
  CUDFPropertyValue(CUDFProperty *the_property, CUDFVpkgFormula *the_value);
};

class CUDFProperty {
 public:
  char *name;

  CUDFPropertyType type_id;
  CUDFEnums *enuml;

  bool required;

  CUDFPropertyValue *default_value;

  CUDFProperty(char *tname, CUDFPropertyType ttype);
  CUDFProperty(char *tname, CUDFPropertyType ttype, int tdefault);
  CUDFProperty(char *tname, CUDFPropertyType ttype, char *tdefault);
  CUDFProperty(char *tname, CUDFPropertyType ttype, CUDFEnums *tenum);
  CUDFProperty(char *tname, CUDFPropertyType ttype, CUDFEnums *tenum, char *tident);
  CUDFProperty(char *tname, CUDFPropertyType ttype, CUDFVpkg *tdefault);
  CUDFProperty(char *tname, CUDFPropertyType ttype, CUDFVpkgList *tdefault);
  CUDFProperty(char *tname, CUDFPropertyType ttype, CUDFVpkgFormula *tdefault);

};


extern CUDFproblem *the_problem;

extern CUDFVersionedPackageList all_packages;
extern CUDFVersionedPackageList installed_packages;
extern CUDFVersionedPackageList uninstalled_packages;

extern CUDFVirtualPackageList all_virtual_packages;

extern int parse_cudf(FILE *input_file);

extern bool op_none_comp(CUDFVersion v1, CUDFVersion v2);
extern bool op_eq_comp(CUDFVersion v1, CUDFVersion v2);
extern bool op_neq_comp(CUDFVersion v1, CUDFVersion v2);
extern bool op_sup_comp(CUDFVersion v1, CUDFVersion v2);
extern bool op_supeq_comp(CUDFVersion v1, CUDFVersion v2);
extern bool op_inf_comp(CUDFVersion v1, CUDFVersion v2);
extern bool op_infeq_comp(CUDFVersion v1, CUDFVersion v2);

typedef bool (*a_compptr)(CUDFVersion, CUDFVersion);

extern a_compptr get_comparator(CUDFPackageOp op);

extern void print_enum(FILE *output, CUDFEnums *enuml);
extern void print_properties(FILE *output, CUDFProperties *properties);

extern void print_versioned_package_with_install(FILE *output, CUDFVersionedPackage *pkg, bool install, bool wasinstalled);
extern void print_versioned_package(FILE *output, CUDFVersionedPackage *pkg, bool wasinstalled);
extern void print_versioned_package_as_installed(FILE *output, CUDFVersionedPackage *pkg, bool wasinstalled);

extern void print_virtual_package(FILE *output, CUDFVirtualPackage *vpkg);

extern void print_problem(FILE *output, CUDFproblem *pbs);

#endif
