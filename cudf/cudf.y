
/*******************************************************/
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010         */
/* Distributed under GPLv3                             */
/*******************************************************/


%{

#include <stdio.h>
#include <string.h>
#include <cudf/cudf.hh>
#include <cudf/cudf_hash_table.hh>

extern int cudflineno;
extern int cudflex (void);

 
void yyerror(const char *str)
{
        fprintf(stderr,"cudf syntax error (line: %d): %s\n", cudflineno, str);
}
 
/*
int yywrap()
{
        return 1;
}
*/

int package_line = 0;

CUDFVersionedPackage *current_package = (CUDFVersionedPackage *)NULL;
int versioned_package_rank = 0;
CUDFproblem *current_problem = (CUDFproblem *)NULL;

CUDFVersionedPackageList all_packages;
CUDFVirtualPackageList all_virtual_packages;
CUDFVersionedPackageList installed_packages;
CUDFVersionedPackageList uninstalled_packages;
CUDFproblem *the_problem = (CUDFproblem *)NULL;

CUDFVpkgFormula *current_vpkgformula = (CUDFVpkgFormula *)NULL;
CUDFVpkgList *current_vpkglist = (CUDFVpkgList *)NULL;
CUDFVpkg *current_vpkg = (CUDFVpkg *)NULL;
CUDFPropertyType current_pt_type = pt_vpkgformula;


CUDFVpkg *build_vpkg(const char *vpkgname, CUDFPackageOp op, CUDFVersion version) {
  return new CUDFVpkg(get_virtual_package(vpkgname), op, version);
}

void build_vpkgformula() {
  if (current_vpkgformula == (CUDFVpkgFormula *)NULL) {
    current_vpkgformula = new CUDFVpkgFormula;
  }
  current_vpkgformula->push_back(current_vpkglist);
  current_vpkglist = (CUDFVpkgList *)NULL;
}

void build_vpkglist(CUDFVpkg *avpkg) {
  if (current_vpkglist == (CUDFVpkgList *)NULL) {
    current_vpkglist = new CUDFVpkgList;
  }
  current_vpkglist->push_back(avpkg);
}

// Only used by provides
void build_veqpkglist(CUDFVpkg *avpkg) {
  CUDFVirtualPackage *vpackage = avpkg->virtual_package;
  CUDFVersion version = avpkg->version;

  if (current_vpkglist == (CUDFVpkgList *)NULL) {
    current_vpkglist = new CUDFVpkgList;
  }
  current_vpkglist->push_back(avpkg);
  switch(avpkg->op) {
  case op_none: 
    vpackage->providers.push_back(current_package); 
    break;
  case op_eq:
    if ((current_package->installed) && (version > vpackage->highest_installed_provider_version))
      vpackage->highest_installed_provider_version = version;
    {
      CUDFVersionedProviderListIterator ivpkgl = vpackage->versioned_providers.find(version);
      if (ivpkgl == vpackage->versioned_providers.end())
	vpackage->versioned_providers.insert(CUDFVersionedProviderList::value_type(version, CUDFProviderList(1, current_package)));
      else
	ivpkgl->second.push_back(current_package);
    }
    break;
  default:
    fprintf(stderr, "CUDF error (line %d): veqpkglist is restricted to = operator.\n", cudflineno);
    exit(-1);
    break;
  }
}

CUDFVersion getversion(const char *svalue) {
  CUDFVersion version = 0;
  if (svalue[0] != '-') {
      sscanf(svalue, "%llu", &version);
      if (version >= 0) return version; // WARNING : should be restricted to > 0
  }
  fprintf(stderr, "Package version must be a <posint>.\n");
  exit(-1);
}

vector<CUDFProperty *> required_properties;

void package_postprocess() {
  if (current_package != (CUDFVersionedPackage *)NULL) {
    CUDFVirtualPackage *vpackage = current_package->virtual_package;
    /* WARNING : package version should be posint
    if (current_package->version == 0) {
      fprintf(stderr, "CUDF error (line %d): Package %s has no version.\n", package_line, current_package->name);
      exit(-1);
    }
    */
    if (vpackage->all_versions.find(current_package) == vpackage->all_versions.end())
      vpackage->all_versions.insert(current_package);
    else {
      fprintf(stderr, "CUDF error (line %d): (package, version) must be unique (see package %s).\n", package_line, current_package->name);
      exit(-1);
    }
    if (current_package->installed) {
      installed_packages.push_back(current_package);
      if (vpackage->highest_installed == (CUDFVersionedPackage *)NULL)
	vpackage->highest_installed = current_package;
      else if (current_package->version > vpackage->highest_installed->version)
	vpackage->highest_installed = current_package;
      if (current_package->provides != (CUDFVpkgList *)NULL)
	for (CUDFVpkgListIterator iop = current_package->provides->begin(); iop != current_package->provides->end(); iop++)
	  if ((*iop)->op == op_eq) {
	    if ((*iop)->virtual_package->highest_installed_provider_version < (*iop)->version)
	      (*iop)->virtual_package->highest_installed_provider_version = (*iop)->version;
	  }
    } else
      uninstalled_packages.push_back(current_package);
    if (current_package->version > vpackage->highest_version)
      vpackage->highest_version = current_package->version;
    for (vector<CUDFProperty *>::iterator prop = required_properties.begin(); prop != required_properties.end(); prop++) {
      bool hasprop = false;
      for (CUDFPropertyValueListIterator pval = current_package->properties.begin(); pval != current_package->properties.end(); pval++)
	if ((*pval)->property == *prop) { hasprop = true; break; }
      if (! hasprop) {
	fprintf(stderr, "CUDF error (line %d): package (%s, %llu) lacks property %s.\n",
		package_line, current_package->name, current_package->version, (*prop)->name);
	exit(-1);
      }
    }
	
  }
}

//#define YYMAXDEPTH 800000
//#define YYMAXDEPTH 80

CUDFEnums *enuml;

void build_enum(char *estr) {
  char *the_enum;

  if ((the_enum = (char *)malloc(strlen(estr)+1)) == (char *)NULL) {
    fprintf(stderr, "CUDF error: can not alloc memory for enum %s.\n", estr);
    exit(-1);
  }
  strcpy(the_enum, estr);

  enuml->push_back(the_enum);
}

char *get_enum(CUDFEnums *e, char *estr) {
  for (CUDFEnumsIterator ei = e->begin(); ei != e->end(); ei++)
    if (strcmp((*ei), estr) == 0) return (*ei);
  return (char *)NULL;
}


extern int is_pstring(char *pname);
int is_pstring(char *pname) {
  CUDFPropertiesIterator p = properties.find(string(pname));
  return ((p != properties.end()) && (p->second->type_id == pt_string));
}

extern int pidenttype(char *pname);
int pidenttype(char *pname) {
  CUDFPropertiesIterator p = properties.find(string(pname));

  if (p == properties.end()) 
    return pt_none;

  return p->second->type_id;
}

CUDFPropertyType gettype(char *ident) {
  /* enum CUDFPropertyType { pt_bool, pt_int, pt_nat, pt_posint, pt_enum, pt_string, */
  int length = strlen(ident);

  if (length >= 1)
    switch (ident[0]) {
    case 'b':
      if ((length == 4) && (ident[1] == 'o') && (ident[2] == 'o') && (ident[3] == 'l') && (ident[4] == '\0')) return pt_bool; 
    case 'e':
      if ((length == 4) && (ident[1] == 'n') && (ident[2] == 'u') && (ident[3] == 'm') && (ident[4] == '\0')) return pt_enum; 
    case 'i':
      if ((length == 3) && (ident[1] == 'n') && (ident[2] == 't') && (ident[3] == '\0')) return pt_int; 
    case 'n':
      if ((length == 3) && (ident[1] == 'a') && (ident[2] == 't') && (ident[3] == '\0')) return pt_nat; 
    case 'p':
      if ((length == 6) && (ident[1] == 'o') && (ident[2] == 's') && (ident[3] == 'i') && (ident[4] == 'n') && (ident[5] == 't') && (ident[6] == '\0')) return pt_posint; 
    case 's':
      if ((length == 6) && (ident[1] == 't') && (ident[2] == 'r') && (ident[3] == 'i') && (ident[4] == 'n') && (ident[5] == 'g') && (ident[6] == '\0')) return pt_string; 
    case 'v':
      if (length >= 4) {
	if (ident[1] == 'p') {
	  if ((ident[2] == 'k') && (ident[3] == 'g'))
	    switch (ident[5]) {
	    case '\0': 
	      return pt_vpkg;
	    case 'l': 
	      if ((length == 9) && (ident[6] == 'i') && (ident[7] == 's') && (ident[8] == 't') && (ident[9] == '\0')) return pt_vpkglist;
	    case 'f': 
	      if ((length == 12) && (ident[6] == 'o') && (ident[7] == 'r') && (ident[8] == 'm')  && (ident[9] == 'u')  
		  && (ident[10] == 'l')  && (ident[11] == 'a') && (ident[12] == '\0')) return pt_vpkgformula;
	    }
	} else if (ident[1] == 'e') {
	  if ((ident[2] == 'q') && (ident[3] == 'p') && (length >= 6) && (ident[4] == 'k') && (ident[5] == 'g'))
	    switch (ident[6]) {
	    case '\0': 
	      return pt_veqpkg;
	    case 'l': 
	      if ((length == 10) && (ident[7] == 'i') && (ident[8] == 's') && (ident[9] == 't') && (ident[10] == '\0')) return pt_veqpkglist;
	    }
	}
      }
    }

  fprintf(stderr, "CUDF error (line %d): property type awaited %s.\n", cudflineno, ident);
  exit(-1);
}

extern char astring[];

%}


%union {
  //  CUDFVersion value;
  CUDFVpkg *avpkg;
  char str[256];
}


%token PREAMBLE PROPERTYDEF
%token PACKAGE VERSION DEPENDS CONFLICTS PROVIDES INSTALLED
%token KEEP KEEPVERSION KEEPPACKAGE KEEPFEATURE KEEPNONE
%token EQ NEQ SUP SUPEQ INF INFEQ
%token TRUE FALSE
%token PROBLEM INSTALL WASINSTALLED REMOVE UPGRADE
%token <str> PIDENT_BOOL PIDENT_INT PIDENT_NAT PIDENT_POSINT PIDENT_ENUM PIDENT_STRING
%token <str> PIDENT_VPKG PIDENT_VEQPKG PIDENT_VPKGLIST PIDENT_VEQPKGLIST PIDENT_VPKGFORMULA
%token <str> INTEGER IDENT PIDENT
%token STRING
%type <avpkg> vpkg veqpkg


%%

cudf:   preambles
      | preambles package_declarations
      | problem_declaration
      | preambles package_declarations problem_declaration
      ;

preambles: /* empty */
           | PREAMBLE property_definitions
           
property_definitions: typedecls
                      | PROPERTYDEF typedecls
                      ;

typedecls: typedecl | typedecls ',' typedecl

typedecl: PIDENT IDENT { CUDFProperty *prop = new CUDFProperty($1, gettype($2)); properties[string($1)] = prop; required_properties.push_back(prop);  }
typedecl: PIDENT IDENT EQ '[' TRUE ']'  { if (gettype($2) == pt_bool) 
	                                    properties[string($1)] = new CUDFProperty($1, pt_bool, 1); 
	                                  else {
					    fprintf(stderr, "CUDF error (line %d): property value requires a boolean typed property (%s).\n", cudflineno, $2);
					    exit(-1);              
					  }
                                        }
typedecl: PIDENT IDENT EQ '[' FALSE ']' { if (gettype($2) == pt_bool) 
	                                    properties[string($1)] = new CUDFProperty($1, pt_bool, 0); 
	                                  else {
					    fprintf(stderr, "CUDF error (line %d): property value requires a boolean typed property (%s).\n", cudflineno, $2);
					    exit(-1);              
					  }
                                        }
typedecl: PIDENT IDENT EQ '[' INTEGER ']' {  CUDFPropertyType pt = gettype($2);
	                                     if (pt == pt_int)  
					       properties[string($1)] = new CUDFProperty($1, pt_int, atoi($5)); 
					     else if (pt == pt_posint)  
					       properties[string($1)] = new CUDFProperty($1, pt_posint, atoi($5)); 
					     else if (pt == pt_nat)  
					       properties[string($1)] = new CUDFProperty($1, pt_nat, atoi($5)); 
					     else {
					       fprintf(stderr, "CUDF error (line %d): property value requires an integer typed property (%s).\n", cudflineno, $2);
					       exit(-1);              
					     }
                                           }
typedecl: PIDENT IDENT EQ '[' STRING ']' { if (gettype($2) == pt_string) 
	                                     properties[string($1)] = new CUDFProperty($1, pt_string, astring); 
	                                    else {
					     fprintf(stderr, "CUDF error (line %d): property value requires a string typed property (%s).\n", cudflineno, $2);
					     exit(-1);              
					    }
                                          }

typedecl: PIDENT IDENT '[' enums ']' { 
  CUDFProperty *prop = new CUDFProperty($1, pt_enum, enuml); 
  if (gettype($2) != pt_enum) {
    fprintf(stderr, "CUDF error (line %d): this must be an enum type (%s).\n", cudflineno, $2);
    exit(-1);              
  }
  properties[string($1)] = prop; 
  required_properties.push_back(prop); 
}

typedecl: PIDENT IDENT '[' enums ']' EQ '[' IDENT ']' { 
  if (gettype($2) == pt_enum) 
    properties[string($1)] = new CUDFProperty($1, pt_enum, enuml, $8); 
  else {
    fprintf(stderr, "CUDF error (line %d): property value requires an enum type (%s).\n", cudflineno, $2);
    exit(-1);              
  }
}

/*
typedecl: PIDENT IDENT EQ '[' vpkg ']' {}
typedecl: PIDENT IDENT EQ '[' veqpkg ']' {}
typedecl: PIDENT IDENT EQ '[' vpkglist ']' {}
typedecl: PIDENT IDENT EQ '[' veqpkglist ']' {}
*/
typedecl: PIDENT IDENT EQ '[' vpkgformula ']' {}


enums: IDENT { enuml = new CUDFEnums; build_enum($1); } 
       | enums ',' IDENT { build_enum($3); }

/*
package_declarations: package_declaration
		     ;
package_declarations: package_declarations package_declaration
		     ;
*/

package_declarations:   package_declaration
                      | package_declarations package_declaration /* Problem here : pushing this state for ever */
		     ;

package_declaration: package_version package_options
		     ;

package_version:  PACKAGE IDENT { 
  package_postprocess();
  package_line = cudflineno;
  CUDFVirtualPackage *virtual_package = get_virtual_package($2);
  current_package = new CUDFVersionedPackage($2, versioned_package_rank++);
  all_packages.push_back(current_package);
  current_package->virtual_package = virtual_package;
}

package_options: /* empty */
                 | package_options package_option
                 ;

package_option: version | depends | conflicts | provides | installed | wasinstalled | keep | property
                ;

problem_declaration: problem problem_actions
		     ;

problem_actions: problem_action | problem_actions problem_action
		 ;

problem_action: install | remove | upgrade
                ;

version: VERSION INTEGER {  current_package->set_version(getversion($2)); }

depends: DEPENDS | DEPENDS vpkgformula { 
  if (current_package->depends == (CUDFVpkgFormula *)NULL) {
    current_package->depends = current_vpkgformula; 
    current_vpkgformula = (CUDFVpkgFormula *)NULL; 
  } else {
    fprintf(stderr, "CUDF error (line %d): depends declared twice for package %s.\n", cudflineno, current_package->name);
    exit(-1);
  }
}

conflicts: CONFLICTS | CONFLICTS vpkglist { 
  if (current_package->conflicts == (CUDFVpkgList *)NULL) {
    current_package->conflicts = current_vpkglist; 
    current_vpkglist = (CUDFVpkgList *)NULL; 
  } else {
    fprintf(stderr, "CUDF error (line %d): conflicts declared twice for package %s.\n", cudflineno, current_package->name);
    exit(-1);
  }
}

provides: PROVIDES | PROVIDES veqpkglist { 
  if (current_package->provides == (CUDFVpkgList *)NULL) {
    current_package->provides = current_vpkglist; 
    current_vpkglist = (CUDFVpkgList *)NULL; 
  } else {
    fprintf(stderr, "CUDF error (line %d): provides declared twice for package %s.\n", cudflineno, current_package->name);
    exit(-1);
  }
}

installed: INSTALLED TRUE { current_package->installed = true; }

installed: INSTALLED FALSE { current_package->installed = false; }

wasinstalled: WASINSTALLED TRUE { current_package->wasinstalled = true; }

wasinstalled: WASINSTALLED FALSE { current_package->wasinstalled = false; }

keep: KEEP keepvalue
      ;

/* properties: property | properties property ; */

property: PIDENT_BOOL TRUE { 
  CUDFPropertiesIterator p = properties.find(string($1));
  if (p == properties.end()) {
    fprintf(stderr, "CUDF error (line %d): property %s is not defined.\n", cudflineno, $1);
    exit(-1);
  }
  switch (p->second->type_id) {
  case pt_bool:
    current_package->properties.push_back(new CUDFPropertyValue(p->second, 1));
    break;
  default:
    fprintf(stderr, "CUDF error (line %d): bad value (true) for property %s.\n", cudflineno, $1);
      exit(-1);
  }
}

property: PIDENT_BOOL FALSE { 
  CUDFPropertiesIterator p = properties.find(string($1));
  if (p == properties.end()) {
    fprintf(stderr, "CUDF error (line %d): property %s is not defined.\n", cudflineno, $1);
    exit(-1);
  }
  switch (p->second->type_id) {
  case pt_bool:
    current_package->properties.push_back(new CUDFPropertyValue(p->second, 0));
    break;
  default:
    fprintf(stderr, "CUDF error (line %d): bad value (false) for property %s.\n", cudflineno, $1);
    exit(-1);
  }
}

property: PIDENT_INT INTEGER { 
  CUDFPropertiesIterator p = properties.find(string($1));
  if (p == properties.end()) {
    fprintf(stderr, "CUDF error (line %d): property %s is not defined.\n", cudflineno, $1);
    print_properties(stdout, &properties);
    exit(-1);
  }
  int value = atoi($2);
  switch (p->second->type_id) {
  case pt_int:
    current_package->properties.push_back(new CUDFPropertyValue(p->second, value));
    break;
  default:
    fprintf(stderr, "CUDF error (line %d): bad value (%d) for property %s.\n", cudflineno, value, $1);
      exit(-1);
  }
}

property: PIDENT_NAT INTEGER { 
  CUDFPropertiesIterator p = properties.find(string($1));
  if (p == properties.end()) {
    fprintf(stderr, "CUDF error (line %d): property %s is not defined.\n", cudflineno, $1);
    print_properties(stdout, &properties);
    exit(-1);
  }
  int value = atoi($2);
  switch (p->second->type_id) {
  case pt_nat:
    if (value < 0) {
      fprintf(stderr, "CUDF error (line %d): property %s (nat) requires values >= 0: %d.\n", cudflineno, $1, value);
      exit(-1);
    }
    current_package->properties.push_back(new CUDFPropertyValue(p->second, value));
    break;
  default:
    fprintf(stderr, "CUDF error (line %d): bad value (%d) for property %s.\n", cudflineno, value, $1);
      exit(-1);
  }
}

property: PIDENT_POSINT INTEGER { 
  CUDFPropertiesIterator p = properties.find(string($1));
  if (p == properties.end()) {
    fprintf(stderr, "CUDF error (line %d): property %s is not defined.\n", cudflineno, $1);
    print_properties(stdout, &properties);
    exit(-1);
  }
  int value = atoi($2);
  switch (p->second->type_id) {
  case pt_posint:
    if (value <= 0) {
      fprintf(stderr, "CUDF error (line %d): property %s (posint) requires values > 0: %d.\n", cudflineno, $1, value);
      exit(-1);
    }
    current_package->properties.push_back(new CUDFPropertyValue(p->second, value));
    break;
  default:
    fprintf(stderr, "CUDF error (line %d): bad value (%d) for property %s.\n", cudflineno, value, $1);
      exit(-1);
  }
}

property: PIDENT_ENUM IDENT { 
  CUDFPropertiesIterator p = properties.find(string($1));
  if (p == properties.end()) {
    fprintf(stderr, "CUDF error (line %d): property %s is not defined.\n", cudflineno, $1);
    exit(-1);
  }
  char *value;
  switch (p->second->type_id) {
  case pt_enum:
    value = get_enum(p->second->enuml, $2);
    if (value == (char *)NULL) {
      fprintf(stderr, "CUDF error (line %d): property %s (enum) can not take value %s.\n", cudflineno, $1, $2);
      exit(-1);
    }
    current_package->properties.push_back(new CUDFPropertyValue(p->second, value));
    break;
  default:
    fprintf(stderr, "CUDF error (line %d): bad value (%s) for property %s.\n", cudflineno, $2, $1);
    exit(-1);
  }
}

property: PIDENT_STRING STRING { 
  CUDFPropertiesIterator p = properties.find(string($1));
  if (p == properties.end()) {
    fprintf(stderr, "CUDF error (line %d): property %s is not defined.\n", cudflineno, $1);
    exit(-1);
  }
  switch (p->second->type_id) {
  case pt_string:
    current_package->properties.push_back(new CUDFPropertyValue(p->second, astring));
    break;
  default:
    fprintf(stderr, "CUDF error (line %d): bad value (%s) for property %s.\n", cudflineno, astring, $1);
    exit(-1);
  }
}

problem: PROBLEM { 
  package_postprocess();
  the_problem = current_problem = new CUDFproblem(); 
}

install: INSTALL vpkglist    { 
  if (current_problem->install == (CUDFVpkgList *)NULL) {
    current_problem->install = current_vpkglist; 
    current_vpkglist = (CUDFVpkgList *)NULL; 
  } else {
    fprintf(stderr, "CUDF error (line %d): install declared twice.\n", cudflineno);
    exit(-1);
  }
}

remove: REMOVE vpkglist     { 
  if (current_problem->remove == (CUDFVpkgList *)NULL) {
    current_problem->remove = current_vpkglist; 
    current_vpkglist = (CUDFVpkgList *)NULL; 
  } else {
    fprintf(stderr, "CUDF error (line %d): remove declared twice.\n", cudflineno);
    exit(-1);
  }
}

upgrade: UPGRADE vpkglist     { 
  if (current_problem->upgrade == (CUDFVpkgList *)NULL) {
    current_problem->upgrade = current_vpkglist; 
    current_vpkglist = (CUDFVpkgList *)NULL; 
  } else {
    fprintf(stderr, "CUDF error (line %d): upgrade declared twice.\n", cudflineno);
    exit(-1);
  }
}

keepvalue: KEEPVERSION { 
  if (current_package->keep == keep_none) {
    current_package->keep = keep_version;
  } else {
    fprintf(stderr, "CUDF error (line %d): keep declared twice for package %s.\n", cudflineno, current_package->name);
    exit(-1);
  }
}

keepvalue: KEEPPACKAGE { 
  if (current_package->keep == keep_none) {
    current_package->keep = keep_package;
  } else {
    fprintf(stderr, "CUDF error (line %d): keep declared twice for package %s.\n", cudflineno, current_package->name);
    exit(-1);
  }
}

keepvalue: KEEPFEATURE {
  if (current_package->keep == keep_none) {
    current_package->keep = keep_version;
  } else {
    fprintf(stderr, "CUDF error (line %d): keep declared twice for package %s.\n", cudflineno, current_package->name);
    exit(-1);
  }
}

keepvalue: KEEPNONE {
  current_package->keep = keep_none;
}


vpkgformula: vpkgor { build_vpkgformula(); }
vpkgformula: vpkgformula ',' vpkgor { build_vpkgformula(); }

vpkgor: vpkg { build_vpkglist($1); }
vpkgor: vpkgor '|' vpkg { build_vpkglist($3); }

veqpkg: IDENT { current_vpkg = $$ = build_vpkg($1, op_none, 0); }
veqpkg: IDENT EQ INTEGER { current_vpkg = $$ = build_vpkg($1, op_eq, getversion($3)); }

vpkg: veqpkg { $$ = $1; }
vpkg: IDENT NEQ INTEGER { current_vpkg = $$ = build_vpkg($1, op_neq, getversion($3)); }
vpkg: IDENT SUP INTEGER { current_vpkg = $$ = build_vpkg($1, op_sup, getversion($3)); }
vpkg: IDENT SUPEQ INTEGER { current_vpkg = $$ = build_vpkg($1, op_supeq, getversion($3)); }
vpkg: IDENT INF INTEGER { current_vpkg = $$ = build_vpkg($1, op_inf, getversion($3)); }
vpkg: IDENT INFEQ INTEGER { current_vpkg = $$ = build_vpkg($1, op_infeq, getversion($3));}


vpkglist: vpkg  { build_vpkglist($1); }
vpkglist: vpkglist ',' vpkg  { build_vpkglist($3); }

veqpkglist: veqpkg { build_veqpkglist($1); }
veqpkglist: veqpkglist ',' veqpkg { build_veqpkglist($3); }



%%

extern FILE *cudfin;

int parse_cudf(FILE *input_file) { 
  //  cudfdebug = 1; 
  cudfin = input_file; return cudfparse(); 
}

