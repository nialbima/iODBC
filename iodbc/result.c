/*
 *  result.c
 *
 *  $Id$
 *
 *  Prepare for getting query result
 *
 *  The iODBC driver manager.
 *  
 *  Copyright (C) 1995 by Ke Jin <kejin@empress.com> 
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>

#include <sql.h>
#include <sqlext.h>

#include <dlproc.h>

#include <herr.h>
#include <henv.h>
#include <hdbc.h>
#include <hstmt.h>

#include <itrace.h>

SQLRETURN SQL_API
SQLBindCol (
    SQLHSTMT hstmt,
    SQLUSMALLINT icol,
    SQLSMALLINT fCType,
    SQLPOINTER rgbValue,
    SQLINTEGER cbValueMax,
    SQLINTEGER FAR * pcbValue)
{
  STMT_t FAR *pstmt = (STMT_t FAR *) hstmt;
  HPROC hproc = SQL_NULL_HPROC;
  SQLRETURN retcode;

  ENTER_STMT (pstmt);

  /* check argument */
  switch (fCType)
    {
    case SQL_C_DEFAULT:
    case SQL_C_CHAR:
    case SQL_C_BINARY:
    case SQL_C_BIT:
    case SQL_C_TINYINT:
    case SQL_C_STINYINT:
    case SQL_C_UTINYINT:
    case SQL_C_SHORT:
    case SQL_C_SSHORT:
    case SQL_C_USHORT:
    case SQL_C_LONG:
    case SQL_C_SLONG:
    case SQL_C_ULONG:
    case SQL_C_FLOAT:
    case SQL_C_DOUBLE:
    case SQL_C_DATE:
    case SQL_C_TIME:
    case SQL_C_TIMESTAMP:
      break;

    default:
      PUSHSQLERR (pstmt->herr, en_S1003);
      LEAVE_STMT (pstmt, SQL_ERROR);
    }

  if (cbValueMax < 0)
    {
      PUSHSQLERR (pstmt->herr, en_S1090);

      LEAVE_STMT (pstmt, SQL_ERROR);
    }

  /* check state */
  if (pstmt->state > en_stmt_needdata || pstmt->asyn_on != en_NullProc)
    {
      PUSHSQLERR (pstmt->herr, en_S1010);
      LEAVE_STMT (pstmt, SQL_ERROR);
    }

  /* call driver's function */
  hproc = _iodbcdm_getproc (pstmt->hdbc, en_BindCol);

  if (hproc == SQL_NULL_HPROC)
    {
      PUSHSQLERR (pstmt->herr, en_IM001);

      LEAVE_STMT (pstmt, SQL_ERROR);
    }

  CALL_DRIVER (pstmt->hdbc, pstmt, retcode, hproc, en_BindCol,
      (pstmt->dhstmt, icol, fCType, rgbValue, cbValueMax, pcbValue));

  LEAVE_STMT (pstmt, retcode);
}


SQLRETURN SQL_API
SQLGetCursorName (
    SQLHSTMT hstmt,
    SQLCHAR FAR * szCursor,
    SQLSMALLINT cbCursorMax,
    SQLSMALLINT FAR * pcbCursor)
{
  STMT_t FAR *pstmt = (STMT_t FAR *) hstmt;
  HPROC hproc;
  SQLRETURN retcode;

  ENTER_STMT (pstmt);

  /* check argument */
  if (cbCursorMax < (SWORD) 0)
    {
      PUSHSQLERR (pstmt->herr, en_S1090);

      LEAVE_STMT (pstmt, SQL_ERROR);
    }

  /* check state */
  if (pstmt->state >= en_stmt_needdata || pstmt->asyn_on != en_NullProc)
    {
      PUSHSQLERR (pstmt->herr, en_S1010);

      LEAVE_STMT (pstmt, SQL_ERROR);
    }

  if (pstmt->state < en_stmt_cursoropen
      && pstmt->cursor_state == en_stmt_cursor_no)
    {
      PUSHSQLERR (pstmt->herr, en_S1015);

      LEAVE_STMT (pstmt, SQL_ERROR);
    }

  /* call driver's function */
  hproc = _iodbcdm_getproc (pstmt->hdbc, en_GetCursorName);

  if (hproc == SQL_NULL_HPROC)
    {
      PUSHSQLERR (pstmt->herr, en_IM001);

      LEAVE_STMT (pstmt, SQL_ERROR);
    }

  CALL_DRIVER (pstmt->hdbc, pstmt, retcode, hproc, en_GetCursorName,
      (pstmt->dhstmt, szCursor, cbCursorMax, pcbCursor));

  LEAVE_STMT (pstmt, retcode);
}


SQLRETURN SQL_API
SQLRowCount (
    SQLHSTMT hstmt,
    SQLINTEGER FAR * pcrow)
{
  STMT_t FAR *pstmt = (STMT_t FAR *) hstmt;
  HPROC hproc;
  SQLRETURN retcode;

  ENTER_STMT (pstmt);

  /* check state */
  if (pstmt->state >= en_stmt_needdata
      || pstmt->state <= en_stmt_prepared
      || pstmt->asyn_on != en_NullProc)
    {
      PUSHSQLERR (pstmt->herr, en_S1010);

      LEAVE_STMT (pstmt, SQL_ERROR);
    }

  /* call driver */
  hproc = _iodbcdm_getproc (pstmt->hdbc, en_RowCount);

  if (hproc == SQL_NULL_HPROC)
    {
      PUSHSQLERR (pstmt->herr, en_IM001);

      LEAVE_STMT (pstmt, SQL_ERROR);
    }

  CALL_DRIVER (pstmt->hdbc, pstmt, retcode, hproc, en_RowCount,
      (pstmt->dhstmt, pcrow));

  LEAVE_STMT (pstmt, retcode);
}


SQLRETURN SQL_API
_iodbcdm_NumResultCols (
    SQLHSTMT hstmt,
    SQLSMALLINT FAR * pccol)
{
  STMT (pstmt, hstmt);
  HPROC hproc;
  SQLRETURN retcode;
  SWORD ccol;

  /* check state */
  if (pstmt->asyn_on == en_NullProc)
    {
      if (pstmt->state == en_stmt_allocated
	  || pstmt->state >= en_stmt_needdata)
	{
	  PUSHSQLERR (pstmt->herr, en_S1010);

	  return SQL_ERROR;
	}
    }
  else if (pstmt->asyn_on != en_NumResultCols)
    {
      PUSHSQLERR (pstmt->herr, en_S1010);

      return SQL_ERROR;
    }

  /* call driver */
  hproc = _iodbcdm_getproc (pstmt->hdbc, en_NumResultCols);

  if (hproc == SQL_NULL_HPROC)
    {
      PUSHSQLERR (pstmt->herr, en_IM001);

      return SQL_ERROR;
    }

  CALL_DRIVER (pstmt->hdbc, pstmt, retcode, hproc, en_NumResultCols,
      (pstmt->dhstmt, &ccol));

  /* state transition */
  if (pstmt->asyn_on == en_NumResultCols)
    {
      switch (retcode)
	{
	case SQL_SUCCESS:
	case SQL_SUCCESS_WITH_INFO:
	case SQL_ERROR:
	  pstmt->asyn_on = en_NullProc;

	case SQL_STILL_EXECUTING:
	default:
	  break;
	}
    }

  switch (retcode)
    {
    case SQL_SUCCESS:
    case SQL_SUCCESS_WITH_INFO:
      break;

    case SQL_STILL_EXECUTING:
      ccol = 0;
      pstmt->asyn_on = en_NumResultCols;
      break;

    default:
      ccol = 0;
      break;
    }

  if (pccol)
    {
      *pccol = ccol;
    }
  return retcode;
}


SQLRETURN SQL_API
SQLNumResultCols (
    SQLHSTMT hstmt,
    SQLSMALLINT FAR * pccol)
{
  STMT (pstmt, hstmt);
  SQLRETURN retcode;

  ENTER_STMT (pstmt);

  retcode = _iodbcdm_NumResultCols (hstmt, pccol);

  LEAVE_STMT (pstmt, retcode);
}


SQLRETURN SQL_API
SQLDescribeCol (
    SQLHSTMT hstmt,
    SQLUSMALLINT icol,
    SQLCHAR FAR * szColName,
    SQLSMALLINT cbColNameMax,
    SQLSMALLINT FAR * pcbColName,
    SQLSMALLINT FAR * pfSqlType,
    SQLUINTEGER FAR * pcbColDef,
    SQLSMALLINT FAR * pibScale,
    SQLSMALLINT FAR * pfNullable)
{
  STMT_t FAR *pstmt = (STMT_t FAR *) hstmt;
  HPROC hproc;
  SQLRETURN retcode;
  int sqlstat = en_00000;

  ENTER_STMT (pstmt);

  /* check arguments */
  if (icol == 0)
    {
      sqlstat = en_S1002;
    }
  else if (cbColNameMax < 0)
    {
      sqlstat = en_S1090;
    }

  if (sqlstat != en_00000)
    {
      PUSHSQLERR (pstmt->herr, sqlstat);

      LEAVE_STMT (pstmt, SQL_ERROR);
    }
#if (ODBCVER < 0x0300)
  /* check state */
  if (pstmt->asyn_on == en_NullProc)
    {
      if (pstmt->state == en_stmt_allocated
	  || pstmt->state >= en_stmt_needdata)
	{
	  sqlstat = en_S1010;
	}
    }
  else if (pstmt->asyn_on != en_DescribeCol)
    {
      sqlstat = en_S1010;
    }
#endif

  if (sqlstat != en_00000)
    {
      PUSHSQLERR (pstmt->herr, sqlstat);

      LEAVE_STMT (pstmt, SQL_ERROR);
    }

  /* call driver */
  hproc = _iodbcdm_getproc (pstmt->hdbc, en_DescribeCol);

  if (hproc == SQL_NULL_HPROC)
    {
      PUSHSQLERR (pstmt->herr, en_IM001);

      LEAVE_STMT (pstmt, SQL_ERROR);
    }

  CALL_DRIVER (pstmt->hdbc, pstmt, retcode, hproc, en_DescribeCol,
      (pstmt->dhstmt, icol, szColName, cbColNameMax, pcbColName,
	  pfSqlType, pcbColDef, pibScale, pfNullable));

  /* state transition */
  if (pstmt->asyn_on == en_DescribeCol)
    {
      switch (retcode)
	{
	case SQL_SUCCESS:
	case SQL_SUCCESS_WITH_INFO:
	case SQL_ERROR:
	  pstmt->asyn_on = en_NullProc;
	  break;

	default:
	   LEAVE_STMT (pstmt, retcode);
	}
    }

  switch (pstmt->state)
    {
    case en_stmt_prepared:
    case en_stmt_cursoropen:
    case en_stmt_fetched:
    case en_stmt_xfetched:
      if (retcode == SQL_STILL_EXECUTING)
	{
	  pstmt->asyn_on = en_DescribeCol;
	}
      break;

    default:
      break;
    }

  LEAVE_STMT (pstmt, retcode);
}


SQLRETURN SQL_API
SQLColAttributes (
    SQLHSTMT hstmt,
    SQLUSMALLINT icol,
    SQLUSMALLINT fDescType,
    SQLPOINTER rgbDesc,
    SQLSMALLINT cbDescMax,
    SQLSMALLINT FAR * pcbDesc,
    SQLINTEGER FAR * pfDesc)
{
  STMT_t FAR *pstmt = (STMT_t FAR *) hstmt;
  HPROC hproc;
  SQLRETURN retcode;
  int sqlstat = en_00000;

  ENTER_STMT (pstmt);

  /* check arguments */
  if (icol == 0 && fDescType != SQL_COLUMN_COUNT)
    {
      sqlstat = en_S1002;
    }
  else if (cbDescMax < 0)
    {
      sqlstat = en_S1090;
    }
#if (ODBCVER < 0x0300)
  else if (			/* fDescType < SQL_COLATT_OPT_MIN || *//* turnoff warning */
	(fDescType > SQL_COLATT_OPT_MAX
	  && fDescType < SQL_COLUMN_DRIVER_START))
    {
      sqlstat = en_S1091;
    }
#endif /* ODBCVER < 0x0300 */

  if (sqlstat != en_00000)
    {
      PUSHSQLERR (pstmt->herr, sqlstat);

      LEAVE_STMT (pstmt, SQL_ERROR);
    }

  /* check state */
  if (pstmt->asyn_on == en_NullProc)
    {
      if (pstmt->state == en_stmt_allocated
	  || pstmt->state >= en_stmt_needdata)
	{
	  sqlstat = en_S1010;
	}
    }
  else if (pstmt->asyn_on != en_ColAttributes)
    {
      sqlstat = en_S1010;
    }

  if (sqlstat != en_00000)
    {
      PUSHSQLERR (pstmt->herr, sqlstat);

      LEAVE_STMT (pstmt, SQL_ERROR);
    }

  /* call driver */
#if (ODBCVER >= 0x0300)

  hproc = _iodbcdm_getproc (pstmt->hdbc, en_ColAttribute);

  if (hproc != SQL_NULL_HPROC)
    {
      SQLUSMALLINT new_attr = fDescType;
      switch (new_attr)
	{
	case SQL_COLUMN_NAME:
	  new_attr = SQL_DESC_NAME;
	  break;
	case SQL_COLUMN_NULLABLE:
	  new_attr = SQL_DESC_NULLABLE;
	  break;
	case SQL_COLUMN_COUNT:
	  new_attr = SQL_DESC_COUNT;
	  break;
	}
      CALL_DRIVER (pstmt->hdbc, pstmt, retcode, hproc, en_ColAttribute,
	  (pstmt->dhstmt, icol, fDescType, rgbDesc, cbDescMax, pcbDesc, pfDesc));
    }
  else
#endif

    {
      hproc = _iodbcdm_getproc (pstmt->hdbc, en_ColAttributes);

      if (hproc == SQL_NULL_HPROC)
	{
	  PUSHSQLERR (pstmt->herr, en_IM001);

	  LEAVE_STMT (pstmt, SQL_ERROR);
	}

      CALL_DRIVER (pstmt->hdbc, pstmt, retcode, hproc, en_ColAttributes,
	  (pstmt->dhstmt, icol, fDescType, rgbDesc, cbDescMax, pcbDesc, pfDesc));
    }

  /* state transition */
  if (pstmt->asyn_on == en_ColAttributes)
    {
      switch (retcode)
	{
	case SQL_SUCCESS:
	case SQL_SUCCESS_WITH_INFO:
	case SQL_ERROR:
	  pstmt->asyn_on = en_NullProc;
	  break;

	default:
	   LEAVE_STMT (pstmt, retcode);
	}
    }

  switch (pstmt->state)
    {
    case en_stmt_prepared:
    case en_stmt_cursoropen:
    case en_stmt_fetched:
    case en_stmt_xfetched:
      if (retcode == SQL_STILL_EXECUTING)
	{
	  pstmt->asyn_on = en_ColAttributes;
	}
      break;

    default:
      break;
    }

  LEAVE_STMT (pstmt, retcode);
}