/*
 * ET <-> ODBC interface header file.
 */

/* TODO: Windows build - Find equiv headers for sql.h and sqlext.h and test */

#ifndef _Q_ODBC_H
#define _Q_ODBC_H

#ifdef FEATURE_ODBC

#include "q_shared.h"
#include "qcommon.h"

// TODO: Find out if this is needed since not using mysql anymore
#ifdef WIN32
#include <windows.h>
#include <winsock.h>
#pragma warning (disable: 4514 4786)
#pragma warning( push, 3 )
#endif

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

#ifndef WIN32
#include <unistd.h>
#endif

// Database connection cvars
extern cvar_t *sv_odbcEnable;
extern cvar_t *sv_odbcDSN;
extern cvar_t *sv_odbcUsername;
extern cvar_t *sv_odbcPassword;

// Database functions
void Com_ODBC_Error(char *fn, SQLHANDLE handle, SQLSMALLINT type);
void Com_ODBC_InitGameTest();
int Com_ODBC_Ready();

#endif /* ifdef FEATURE_ODBC */

#endif /* ifndef _Q_ODBC_H */

