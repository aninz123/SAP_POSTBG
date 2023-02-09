/********************************************************************************
 * Module ID    : BLC0080501
 * Module Name  : SAP_POST_BG (Revenue Extraction)
 * EXE Name     : BLC0080501_SAP_POST_BG
 * Purpose      : This extraction program is to summarize and distribute 
 *                invoice into region after each bill cycle
 * 
 * Last Revised : 8 September 2003
 *              : Niphon watcharaphalakorn
*  2003/11/09  : Supannika(Arjaree) Mathong 
*                         :Modify BLC0080501 SAP Post BG for support CCC project
*  Last Revised : 8 April 2008
*
*				: Jagdeep S Virdi
*				: AIS Upgrade from Geneva 5.0 to IRB 3.0 (No Impact)
*				: Version Identification Added.
*20081125	: Saowanit   Rungchatsataporn
					: Modify process and Dynamic query
					: Set Version 2.00.00
*********************************************************************************/
#define OTL_ORA10G_R2 // Upgrade to IRB 3.0

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <otlv4.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include "bl_utils.h"
#include "bl_sap_postbg_param.h"
#include "config.h"

otl_connect db; // connection variable to databases

char env_Config_File[MAX_BUFFER];
char str_cfg_Config_File[MAX_BUFFER];
char str_global_config_file[MAX_BUFFER] ;
char str_GNVdbconn[MAX_BUFFER];
char str_log_path[MAX_BUFFER];
char str_log_name[MAX_BUFFER];
char str_msglog[20000];

// Global Parameter
int int_V_GROUP_ID	= 0;
int int_V_NEW_GROUP_ID= 0; 
int int_V_ID1	= 0;
int int_V_ID2	= 0;
int int_V_ICOID	= 0;
int int_V_Region	= 0;
int int_Status	= BL_SAP_POSTBG_OK;

long long_V_TotalRevenue	= 0;
char str_V_ExtractSeq[11];
// RBM 9.0.4 Upgrade
// Satvashila Nikam : Changed the array size for invoicing company name
// RBM 9.0, length of invoicing_co_name has been changed from 40 to 120 chars
char str_V_ICOName[121];
char str_V_VatDate[21];
char str_V_ExtractDat[21];
char str_V_BillType[41];
char str_V_RevType[11];
char str_sqlstmt[MAX_BUFFER];

// CUR SAP structure
int  int_CUR_SAP_SAP_Ext_Group_ID;
int	 int_CUR_SAP_Revenue_Code_ID;
char str_CUR_SAP_Revenue_Code_Name[41];
long long_CUR_SAP_Orig_Revenue_MNY;
int  int_CUR_SAP_Region_Criteria;
char char_CUR_SAP_Derived_Ratio_Used;
char str_CUR_SAP_Revenue_Type[11];
char str_CUR_SAP_Extraction_SEQ[11];

// Add by SIR 7082
int int_fwd_status;
// File Pointer
FILE *file_logfp;
/************************************************************************
 * Function: GetDateTime
 * Purpose:  Get current date and time and copy into string
 * Input:    char: string output
 *			 char: format of date and time to return
 * Output:   None
 * Version/Programmer/Remarks:
 ************************************************************************/
void GetDateTime(char* str_DT, char* format)
{
	time_t		rawtime;
	struct tm*	timeinfo;

	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	strftime(str_DT,30,format,timeinfo);
}
/************************************************************************
 * Function: RoundUp2
 * Purpose:  round 2 digits to nearest value
 * Input:    double : Input to round
 * Output:   double : 100.114 -> 100.11
 *					  100.115 -> 100.12
 * Version/Programmer/Remarks:
 ************************************************************************/
double RoundUp2(double d)
{
	double d1, d2, d3;

	d1 = d * 100;
	d2 = floor(d1) * 10;
	d3 = ( d1 * 10 ) - d2;
	if( d3 > 4 )
		return ceil(d1)/100;

	return d2/1000;
}
/************************************************************************
 * Function: SplitRevenue
 * Purpose:  Split each input revenue delimit by comma
 * Input:    string (char *) : Input to split 152,431
 * Output:   string (char *) : string 1 -> 152
 *						       string 2 -> 431
 * Version/Programmer/Remarks:
 ************************************************************************/
int SplitRevenue(char *str_input, char *str_output1, char *str_output2)
{
	char *str_delim;

	str_delim = strstr(str_input, (char *)",");

	if( str_delim == NULL )
	{
		return(BL_SAP_POSTBG_NOTOK);
	}

	strncpy(str_output1, str_input, strlen(str_input) - strlen(str_delim));
	strcpy(str_output2, str_delim+1);

	return(BL_SAP_POSTBG_OK);
}
/************************************************************************
 * Function: PrintAndLog
 * Purpose:  Print message and also Log to log file
 * Input:    int  Message Type [Error | Message | Warning]
			 char Submodule
			 char Message
 * Output:   None
 * Version/Programmer/Remarks:
 ************************************************************************/
void PrintAndLog(int int_msgtype, char *str_submodule, char *str_message)
{
	switch(int_msgtype)
	{
		case BLERROR:
			printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, str_submodule,str_message);
			bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,str_submodule,str_message);
			break;
		case BLMESSAGE:
			printGenevaMessage("INFORM",SAP_POSTBG_PROG_NAME, str_submodule,str_message);
			bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG, str_submodule,str_message);
			break;
		case BLWARNING:
			printGenevaMessage("WARN",SAP_POSTBG_PROG_NAME, str_submodule,str_message);
			bl_Log_Print(file_logfp,BLSAPPOSTBG_WARNING_LOG, str_submodule,str_message);
			break;
		default:
			printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, (char *)"PrintAndLog",(char *)"Message Type to pring log file");
			bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG, (char *)"PrintAndLog",(char *)"Message Type to pring log file");
	}
}
/************************************************************************
 * Function: bl_sap_postbg_getconfig
 * Purpose:  read config data matching with key
 * Input:    char Key
			 char Return config value
			 char Error Message in case of cannot get config
 * Output:   None
 * Version/Programmer/Remarks:
 ************************************************************************/
void bl_sap_postbg_getconfig(char *str_key, char *str_cfg, char *str_errmsg)
{
	char *charp_paramvalue = NULL;

	if ( (charp_paramvalue = getconfigvalue(str_key)) == NULL)
	{
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "bl_sap_postbg_getparameter",str_errmsg);
		exit(EXIT_FAIL);
	}
	else {
		strcpy(str_cfg, charp_paramvalue);
	}
}
/************************************************************************
 * Function: bl_sap_postbg_getparameter
 * Purpose:  Get parameters from the config file
 * Input:    None
 * Output:   int 0	-> success
 *			     1	-> unsuccess
 * Version/Programmer/Remarks:
 ************************************************************************/
int bl_sap_postbg_getparameter() 
{ 
	int int_status	= BL_SAP_POSTBG_OK;

	// format the location of the configuration file
	// Anil K Dandu 7/25/2008 GNV_BATCH replace commented the old code and added the new code
	if (getgnvbatch(env_Config_File) != 0)
	{
		// Environment variable not found
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME,"bl_sap_postbg_getparameter",ERR_MSG_1);
		int_status = BL_SAP_POSTBG_NOTOK;
	}
	else
	{
	   strncpy(str_cfg_Config_File, env_Config_File,MAX_BUFFER);
	   strcat(str_cfg_Config_File, BLSAPPOSTBG_CONFIGFILE);
	}
	
	//------------------------------------------------------------------------
	if (openconfig(str_cfg_Config_File) == BL_SAP_POSTBG_NOTOK)
	{
		strncpy(str_msglog, ERR_MSG_2,MAX_BUFFER);
		strcat(str_msglog, str_cfg_Config_File);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "bl_sap_postbg_getparameter",ERR_MSG_2);
	
		return BL_SAP_POSTBG_NOTOK;
	}

	bl_sap_postbg_getconfig((char *)GLOBAL_CONFIG_FILE, str_global_config_file, (char *)ERR_MSG_3);
	bl_sap_postbg_getconfig((char *)LOG_PATH, str_log_path, (char *)ERR_MSG_4);
	bl_sap_postbg_getconfig((char *)LOG_FILE, str_log_name, (char *)ERR_MSG_5);	

	// Open configuration file
    if (openconfig(str_global_config_file) == 1)
	{	
		strncpy(str_msglog, ERR_MSG_1,MAX_BUFFER);
		strcat(str_msglog, str_global_config_file);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "bl_sap_postbg_getparameter",str_msglog);

		int_status = BL_SAP_POSTBG_NOTOK;
	}

	// Get Oracle Login Stirng
	bl_sap_postbg_getconfig((char *)GENEVA_DBCONN, str_GNVdbconn, (char *)ERR_MSG_1);

  return int_status;
}
/************************************************************************
 * Function: bl_sap_postbg_RAISE_ERROR
 * Purpose:  update status to 'FAILED' in case of error in extraction
 * Input:    None
 * Output:	 None
 * Version/Programmer/Remarks:
 ************************************************************************/
void bl_sap_postbg_RAISE_ERROR()
{
	// set status flag = 1 in case of any error occur.
	int_fwd_status = 1;

	sprintf(str_msglog,"RAISE ERROR: Extraction Date[%s] Extraction Sequence[%s]",str_V_ExtractDat, str_V_ExtractSeq);
	PrintAndLog(BLERROR, (char *)"RAISE ERROR", str_msglog);

	try
	{
		otl_stream stream_update_failed(1,SQL_UPDATE_SAP_AUDIT_REV_EXT, db);
		stream_update_failed << str_V_ExtractDat << long_V_TotalRevenue << "FAILED" << str_V_ExtractSeq;
		
	}catch(otl_exception& p){ // intercept OTL exceptions

		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.msg);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.stm_text);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.var_info);

		sprintf(str_msglog,"SQL ERROR [%s]",(char*)p.stm_text);
		bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"RAISE ERROR",str_msglog);
	} // end catch

	int_Status = BL_SAP_POSTBG_NOTOK;
}
/************************************************************************
 * Function: bl_sap_postbg_UPDATE_SUCCESS
 * Purpose:  update status to 'SUCCESS' when finish extraction
 * Input:    None
 * Output:	 None
 * Version/Programmer/Remarks:
 ************************************************************************/
void bl_sap_postbg_UPDATE_SUCCESS()
{
	try
	{
		if( int_Status == BL_SAP_POSTBG_OK )
		{
			sprintf(str_msglog,"UPDATE SUCCESS: Extraction Date[%s] Extraction Sequence[%s]",str_V_ExtractDat, str_V_ExtractSeq);
			PrintAndLog(BLMESSAGE, (char*)"UPDATE SUCCESS", str_msglog);
			
			otl_stream stream_update_success(1, SQL_UPDATE_SAP_AUDIT_REV_EXT_SUCCESS, db);
			stream_update_success << str_V_ExtractDat << long_V_TotalRevenue << "SUCCESS" << str_V_ExtractSeq;			
		}
	}catch(otl_exception& p){ // intercept OTL exceptions

		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.msg);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.stm_text);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.var_info);

		sprintf(str_msglog,"SQL ERROR [%s]",(char*)p.stm_text);
		bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"UPDATE_SUCCESS",str_msglog);
	} // end catch
}
/************************************************************************
 * Function: bl_sap_postbg_SQL_1_4
 * Purpose:  Execute SQL 1-4
 * Input:    None
 * Output:	 Int : BL_SAP_POSTBG_OK
				   BL_SAP_POSTBG_NOTOK
 * Version/Programmer/Remarks:
 ************************************************************************/
int bl_sap_postbg_SQL_1_4()
{	
	int int_Tmp_Seq				= -1;
	int int_V_Invoicing_CO_ID	= 0;
	long long_Total_Invoice		= 0;

	char str_V_VatDate[41]		= "";
	char str_V_BillType[41]		= "";
	char str_TmpYr[3]			= "";
	
	int int_count_line=0;

	try
	{
		otl_stream stream_get_inv_list(1024, SQL_SELECT_1, db); //kittipop 21/10/2008
		stream_get_inv_list	<< int_V_GROUP_ID;

		sprintf(str_msglog,"Get Invoice List to extract");
		PrintAndLog(BLMESSAGE, (char*)"SQL_1_4()", str_msglog);
		
		if( stream_get_inv_list.eof() )
		{
			sprintf(str_msglog,"No Invoice List to extract");
			PrintAndLog(BLMESSAGE, (char*)"SQL_1_4()", str_msglog);
			exit(EXIT_SUCCESS);
		}

		while (!stream_get_inv_list.eof())
		{
			stream_get_inv_list >> long_Total_Invoice >> str_V_VatDate >> str_V_BillType >> int_V_Invoicing_CO_ID;
			
			otl_stream	stream_get_nxt_seq(1024, SQL_SELECT_2, db);
			if( !stream_get_nxt_seq.eof() ) 
			{
				stream_get_nxt_seq >> int_Tmp_Seq;
			}

			otl_stream stream_select_yr(1024, SQL_SELECT_2_1, db);			
			if( !stream_select_yr.eof() )
			{
				stream_select_yr >> str_TmpYr;
			}

			if( int_Tmp_Seq == atoi(str_TmpYr) )  //If same year
			{
				otl_stream	stream_get_max_seq(1024, SQL_SELECT_3, db);
				stream_get_max_seq >> int_Tmp_Seq;
				sprintf(str_V_ExtractSeq,"%.7d",int_Tmp_Seq);
			}
			else
			{
				sprintf(str_V_ExtractSeq,"0000001");
			}
			
			otl_stream stream_insert_aud_rev_ext(1, SQL_INSERT_4, db); //kittipop 21/10/2008
			stream_insert_aud_rev_ext	<< int_V_NEW_GROUP_ID
										<< str_V_ExtractSeq
										<< int_V_Invoicing_CO_ID
										<< long_Total_Invoice
										<< str_V_VatDate
										<< str_V_BillType;	
				int_count_line++;													
		} // END WHILE
		
		sprintf(str_msglog,"Finish insert Data into table CC_TBL_DAT_SAP_AUDIT_REV_EXT: SAP_EXT_GROUP_ID = %d and STATUS = PENDING ,Total %d lines",int_V_NEW_GROUP_ID,int_count_line);
		bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"SQL_1_4()",str_msglog);
		db.commit();  //saowanir:20081124
		
	}catch(otl_exception& p){ // intercept OTL exceptions
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.msg);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.stm_text);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.var_info);

		sprintf(str_msglog,"SQL ERROR [%s]",(char*)p.stm_text);
		bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"SQL_1_4()",str_msglog);

		return(BL_SAP_POSTBG_NOTOK);
	} // end catch

	return(BL_SAP_POSTBG_OK);
}
// END SQL 1-4 CUR_DATA
/************************************************************************
 * Function: bl_sap_postbg_SQL_6_8
 * Purpose:  Execute SQL 6-8
 * Input:    None
 * Output:	 Int : BL_SAP_POSTBG_OK
				   BL_SAP_POSTBG_NOTOK
 * Version/Programmer/Remarks:
 ************************************************************************/
int bl_sap_postbg_SQL_6_8()
{
	char char_V_Ratio; 
	int  int_Revenue_Code_ID		= 0;
	long long_Tot_Revenue			= 0;
	char str_Revenue_Code_name[41]	= "";
	int int_count=0;

	try{
		otl_stream stream_select_iconame(1024, SQL_SELECT_5_1, db);
		stream_select_iconame << int_V_ICOID;
	
		sprintf(str_sqlstmt,SQL_SELECT_5_1,int_V_ICOID);
		if( stream_select_iconame.eof() )
		{
			sprintf(str_msglog,ERR_MSG_10, int_V_ICOID, str_sqlstmt);
			PrintAndLog(BLERROR, (char*)"MAP ICONAME", str_msglog);
	
			int_Status = BL_SAP_POSTBG_NOTOK;
			return(int_Status);
		}

		stream_select_iconame >> str_V_ICOName;

		if( strcmp(str_V_BillType,"PERIODIC") == 0 )
		{
			int_V_ID1 = 1;
			int_V_ID2 = 1;
		}
		else
		{
			int_V_ID1 = 4;
			int_V_ID2 = 7;
		}
		
		otl_stream stream_select_rev_list(1024, SQL_SELECT_6, db);
		stream_select_rev_list << int_V_ID1 << int_V_ID2 << int_V_GROUP_ID << str_V_VatDate << str_V_ICOName;
		
		//sprintf(str_sqlstmt,SQL_SELECT_6, int_V_ID1, int_V_ID2, int_V_GROUP_ID, str_V_VatDate, str_V_ICOName);
		if( stream_select_rev_list.eof() )
		{
			// Change on Jan 22, 2004
			// No error when nothing in bill details
			sprintf(str_msglog,"No data found in bill details: mark complete");
			bl_Log_Print(file_logfp,BLSAPPOSTBG_WARNING_LOG,"Retrieve Bill Details",str_msglog);

			int_Status = BL_SAP_POSTBG_HANDLE;
			return(int_Status);
		}

		while(!stream_select_rev_list.eof())
		{
			stream_select_rev_list >> long_Tot_Revenue >> int_Revenue_Code_ID >> str_Revenue_Code_name;
						
			if( long_Tot_Revenue == NULL )
			{
				long_Tot_Revenue = 0;
			}

			long_V_TotalRevenue = long_V_TotalRevenue + long_Tot_Revenue;
				
			// prepare SQL 7
			sprintf(str_sqlstmt,SQL_SELECT_7,int_V_ICOID,int_Revenue_Code_ID);

			otl_stream stream_select_ratio_region(1024, SQL_SELECT_7, db);
			stream_select_ratio_region << int_V_ICOID << int_Revenue_Code_ID;
				
			if(!stream_select_ratio_region.eof())
			{
				// Found Derived_Ratio_Used, Region_ID and Revenue_Type
				stream_select_ratio_region >> char_V_Ratio >> int_V_Region >> str_V_RevType;
	
				otl_stream stream_insert_audit_rev(1, SQL_INSERT_8, db);
				stream_insert_audit_rev << int_V_NEW_GROUP_ID
													<< int_Revenue_Code_ID
													<< str_Revenue_Code_name
													<< long_Tot_Revenue
													<< int_V_Region
													<< char_V_Ratio
													<< str_V_RevType
													<< str_V_ExtractSeq;							
													
					int_count++;																
			}
			else 
			{
				// Not Found
				sprintf(str_msglog, ERR_MSG_12, int_V_ICOID,int_Revenue_Code_ID, str_sqlstmt);
				PrintAndLog(BLERROR, (char*)"SQL7", str_msglog);

				int_Status = BL_SAP_POSTBG_NOTOK;
				return(int_Status);
			}
		}//while
		
		sprintf(str_msglog,"Finish insert Data into table CC_TBL_DAT_SAP_AUDIT_REVCODE : Total %d lines",int_count);
		bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"SQL_6_8",str_msglog);
		db.commit();  //saowanir:20081124
		
	}catch(otl_exception& p){ // intercept OTL exceptions

		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.msg);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.stm_text);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.var_info);

		sprintf(str_msglog,"SQL ERROR [%s]",(char*)p.stm_text);
		bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"SQL 6-8",str_msglog);
	
		int_Status = BL_SAP_POSTBG_NOTOK;
		return(int_Status);
	} // end catch

	return(BL_SAP_POSTBG_OK);
}
// END SQL 6-8 CUR_REVCODE
/************************************************************************
 * Function: bl_sap_postbg_SQL_11_12
 * Purpose:  Execute SQL 11_12
 * Input:    None
 * Output:	 Int : BL_SAP_POSTBG_OK
				   BL_SAP_POSTBG_NOTOK
 * Version/Programmer/Remarks:
 ************************************************************************/
int bl_sap_postbg_SQL_11_12()
{
	char str_RegionCode[11]	= "";
	long long_Tot_Revenue	= 0;
	int int_cnt_upd=0;
	int int_cnt_inst=0;

	try{
		//sprintf(str_sqlstmt,SQL_SELECT_11_1,str_V_ExtractSeq);
		otl_stream stream_select_de_regcode(1024, SQL_SELECT_11_1, db);
		stream_select_de_regcode << str_V_ExtractSeq;

		while(!stream_select_de_regcode.eof())
		{
			stream_select_de_regcode >> str_RegionCode;

			otl_stream stream_select_de_totrev(1024, SQL_SELECT_11_2, db);
			stream_select_de_totrev << str_V_ExtractSeq << str_RegionCode;

			if(!stream_select_de_totrev.eof())
			{
				stream_select_de_totrev >> long_Tot_Revenue;
			}
			else
			{
				long_Tot_Revenue = 0;

				sprintf(str_msglog,"Total Revenue not found");
				bl_Log_Print(file_logfp,BLSAPPOSTBG_WARNING_LOG,"SQL11_2",str_msglog);
			}

			otl_stream stream_select_seq_reg(1024, SQL_SELECT_12, db);
			stream_select_seq_reg << str_V_ExtractSeq << str_RegionCode;

			if(!stream_select_seq_reg.eof())
			{
				otl_stream stream_update_seq_reg(1, SQL_UPDATE_12_1, db);
				stream_update_seq_reg << int_V_NEW_GROUP_ID << long_Tot_Revenue 
														<< str_RegionCode << str_V_ExtractSeq;
														
				int_cnt_upd++;						
			}
			else
			{
				otl_stream stream_insert_seq_reg(1, SQL_INSERT_12_2, db);			
				stream_insert_seq_reg << int_V_NEW_GROUP_ID<< str_V_ExtractSeq
													<< str_RegionCode << long_Tot_Revenue;
			
				int_cnt_inst++;			
			}
		}// end while
		
			sprintf(str_msglog,"Finish INSERT %d lines and UPDATE %d lines: Region money in table CC_TBL_DAT_SAP_AUDIT_RATIO",int_cnt_inst,int_cnt_upd);
			bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"SQL_11_12",str_msglog);
			db.commit();				
	}catch(otl_exception& p){ // intercept OTL exceptions

		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.msg);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.stm_text);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.var_info);

		sprintf(str_msglog,"SQL ERROR [%s]",(char*)p.stm_text);
		bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"SQL 11-12",str_msglog);
		
		return(BL_SAP_POSTBG_NOTOK);
	} // end catch

	return(BL_SAP_POSTBG_OK);
}
// END SQL 11-12 CUR_REGION
/************************************************************************
 * Function: bl_sap_postbg_case_1RE
 * Purpose:  CASE REGION_CRITERIA = '1' AND REVENUE_TYPE = 'RE' Execute SQL 14, 15.1, AND 15.2
 * Input:    None
 * Output:	 Int : BL_SAP_POSTBG_OK
				   BL_SAP_POSTBG_NOTOK
 * Version/Programmer/Remarks:
 ************************************************************************/
int bl_sap_postbg_case_1RE()
{
	char str_V_REGION[3]	= "";
	long long_V_NEWMNY		= 0;
	long long_V_ORIGMNY		= 0;
	int	 int_TMP_LENGTH		= 0;

	try{
		sprintf(str_msglog,"CASE REGION_CRITERIA = 1 AND REVENUE_TYPE = RE");
		bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"Case 1 RE",str_msglog);

		// Copy Region Code
		int_TMP_LENGTH = strlen(str_CUR_SAP_Revenue_Code_Name);
		str_V_REGION[0] = str_CUR_SAP_Revenue_Code_Name[int_TMP_LENGTH-2];
		str_V_REGION[1] = str_CUR_SAP_Revenue_Code_Name[int_TMP_LENGTH-1];
		str_V_REGION[2] = '\0';

		long_V_NEWMNY = long_CUR_SAP_Orig_Revenue_MNY;

		// prepare SQL 14
		//sprintf(str_sqlstmt,SQL_SELECT_14, str_V_ExtractSeq, str_V_REGION, int_CUR_SAP_Revenue_Code_ID);

		// SQL 14
		otl_stream stream_select_exist_1re(1024, SQL_SELECT_ORIGMNY_SAP_AUDIT_BYREGION, db);
		stream_select_exist_1re<< str_V_ExtractSeq<< str_V_REGION<< int_CUR_SAP_Revenue_Code_ID;

		if(!stream_select_exist_1re.eof())
		{			
			// Found Update
			stream_select_exist_1re >> long_V_ORIGMNY;
			
			long_V_NEWMNY = long_V_NEWMNY + long_V_ORIGMNY;

			otl_stream stream_update_1re(1, SQL_UPDATE_SAP_AUDIT_BYREGION, db);
			stream_update_1re<< int_V_NEW_GROUP_ID<< long_V_NEWMNY<< long_V_NEWMNY
											<< long_V_NEWMNY<< str_V_ExtractSeq<< str_V_REGION
											<< int_CUR_SAP_Revenue_Code_ID;

			sprintf(str_msglog,"Update money in table CC_TBL_DAT_SAP_AUDIT_BYREGION");
			bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"Case 1 RE SQL15_1",str_msglog);
		}
		else
		{
			otl_stream stream_insert_1re(1, SQL_INSERT_SAP_AUDIT_BYREGION, db);
			stream_insert_1re <<int_V_NEW_GROUP_ID<< int_CUR_SAP_Revenue_Code_ID
										<< str_V_REGION<< long_V_NEWMNY<< long_V_NEWMNY
										<<  long_V_NEWMNY<< str_V_ExtractSeq;
  			
			sprintf(str_msglog,"Insert money into table CC_TBL_DAT_SAP_AUDIT_BYREGION");
			bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"Case 1 RE SQL15_2",str_msglog);
		}
	}catch(otl_exception& p){ // intercept OTL exceptions

		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.msg);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.stm_text);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.var_info);

		sprintf(str_msglog,"SQL ERROR [%s]",(char*)p.stm_text);
		bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"Case 1 RE",str_msglog);
		
		return(BL_SAP_POSTBG_NOTOK);
	} // end catch
	return(BL_SAP_POSTBG_OK);
}
// END CASE REGION_CRITERIA = '1' AND REVENUE_TYPE = 'RE'
/************************************************************************
 * Function: bl_sap_postbg_case_2RE
 * Purpose:  CASE REGION_CRITERIA = '2' AND REVENUE_TYPE = 'RE'
 * Input:    None
 * Output:	 Int : BL_SAP_POSTBG_OK
				   BL_SAP_POSTBG_NOTOK
 * Version/Programmer/Remarks:
 ************************************************************************/
int bl_sap_postbg_case_2RE()
{
	try{
		int  int_TMP_SAP_Ext_Group_ID	= 0;
		char str_V_TmpExtractSeq[11]	= "";
		char str_V_REGION[11]			= "";
		char str_V_REGIONVALUE[21]			= "";
		
		long long_REGIONMNY				= 0;
		long long_V_NEWMNY				= 0;
		long long_V_REGIONTOTAL			= 0;
		long long_V_REGIONVALUE			= 0;
		double double_TMP_MNY			= 0;
		
		int int_cnt_upd=0;
		int int_cnt_inst=0;

		sprintf(str_msglog,"CASE REGION_CRITERIA = 2 AND REVENUE_TYPE = RE");
		bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"Case 2 RE",str_msglog);

		sprintf(str_sqlstmt,SQL_SELECT_16, str_V_ExtractSeq);

		otl_stream stream_select_sum_reg_mny(1024, SQL_SELECT_16, db);
		stream_select_sum_reg_mny<< str_V_ExtractSeq;

		if(!stream_select_sum_reg_mny.eof())
		{
			stream_select_sum_reg_mny >> long_V_REGIONTOTAL;
		}
		else
		{
			sprintf(str_msglog, ERR_MSG_14, str_sqlstmt);
			printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "Case 2 RE SQL 16 [14]",str_msglog);
			bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"Case 2 RE SQL 16 [14]",str_msglog);

			int_Status = BL_SAP_POSTBG_NOTOK;
			return(int_Status);
		}

		// prepare SQL 17
		sprintf(str_sqlstmt,SQL_SELECT_17, str_V_ExtractSeq);

		// SQL 17
		otl_stream stream_select_all_regratio(1024, SQL_SELECT_17, db);
		stream_select_all_regratio << str_V_ExtractSeq;

		if( stream_select_all_regratio.eof() )
		{
			sprintf(str_msglog, ERR_MSG_15,str_sqlstmt);
			printGenevaMessage("WARNING",SAP_POSTBG_PROG_NAME, "Case 2 RE SQL 17 [15]",str_msglog);
			bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"Case 2 RE SQL 17 [15]",str_msglog);
 
             if ( int_V_ICOID == 2 )
             	{
                            sprintf(str_V_REGION ,"%s", "AIS_DEFAULTREGION");
               }
			else
			{	
                            sprintf(str_V_REGION ,"%s", "DPC_DEFAULTREGION");
              }              
                      
		        sprintf(str_sqlstmt,SQL_SELECT_17_1, str_V_REGION);
		        
		        otl_stream stream_select_default_region(1024, SQL_SELECT_17_1, db);
		        stream_select_default_region << str_V_REGION;
		        
		        if( stream_select_default_region.eof() )
		        {
			    sprintf(str_msglog, "Not found DAFAULT REGION VALUE", str_sqlstmt);
			    printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "Case 2 RE SQL 17_1 [15]",str_msglog);
			    bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"Case 2 RE SQL 17_1 [15]",str_msglog);
			    int_Status = BL_SAP_POSTBG_NOTOK;
			    return(int_Status); 
                    }
                    
                    stream_select_default_region >> long_V_REGIONVALUE;
                       // printf ("long_V_REGIONVALUE = <%ld>", long_V_REGIONVALUE);

                    sprintf (str_V_REGIONVALUE,"%.2ld",long_V_REGIONVALUE);
                     //printf ("******str_V_REGIONVALUE = <%s>*****", str_V_REGIONVALUE);
                     
		        long_V_NEWMNY = long_CUR_SAP_Orig_Revenue_MNY;
			
			//sprintf(str_sqlstmt,SQL_INSERT_17_2, str_V_NEW_GROUP_ID, int_CUR_SAP_Revenue_Code_ID, str_V_REGIONVALUE, long_V_NEWMNY, long_V_NEWMNY, long_V_NEWMNY, str_V_ExtractSeq);
			//otl_stream stream_insert_2re_ratio(1, str_sqlstmt, db);	
			
			otl_stream stream_insert_2re_ratio(1, SQL_INSERT_SAP_AUDIT_BYREGION, db);	
			stream_insert_2re_ratio<<  int_V_NEW_GROUP_ID<< int_CUR_SAP_Revenue_Code_ID
												<<  str_V_REGIONVALUE<< long_V_NEWMNY<< long_V_NEWMNY
												<< long_V_NEWMNY<< str_V_ExtractSeq;
	
			sprintf(str_msglog,"Insert money into table CC_TBL_DAT_SAP_AUDIT_BYREGION to Default Region");
			bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"Case 2 RE SQL17_2",str_msglog);
		}else {
					
		while(!stream_select_all_regratio.eof())
		{
			stream_select_all_regratio >> int_TMP_SAP_Ext_Group_ID >> str_V_TmpExtractSeq >> str_V_REGION >> long_REGIONMNY;

			// check divide by zero
			if( long_V_REGIONTOTAL == 0 )
			{
				sprintf(str_msglog, ERR_MSG_16);
				printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "Case 2 RE SQL 17 [16]",str_msglog);
				bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"Case 2 RE SQL 17 [16]",str_msglog);

				int_Status = BL_SAP_POSTBG_NOTOK;
				return(int_Status);
			}

			// round up ratio money here			 
			double_TMP_MNY = (double)long_REGIONMNY / (double)long_V_REGIONTOTAL;
			double_TMP_MNY = double_TMP_MNY * (double)long_CUR_SAP_Orig_Revenue_MNY;
			double_TMP_MNY = double_TMP_MNY / 1000;
			double_TMP_MNY = RoundUp2(double_TMP_MNY);
			long_V_NEWMNY = (long)(double_TMP_MNY * 1000);

			// prepare SQL 18
			//sprintf(str_sqlstmt,SQL_SELECT_18, str_V_ExtractSeq, str_V_REGION, int_CUR_SAP_Revenue_Code_ID);

			// SQL 18
			otl_stream stream_select_exist_2re(1024, SQL_SELECT_ORIGMNY_SAP_AUDIT_BYREGION, db);
			stream_select_exist_2re<<str_V_ExtractSeq << str_V_REGION<< int_CUR_SAP_Revenue_Code_ID;

			if(!stream_select_exist_2re.eof())
			{
				// Found Update
				long long_V_ORIGMNY = 0;

				stream_select_exist_2re >> long_V_ORIGMNY;
				long_V_NEWMNY = long_V_NEWMNY + long_V_ORIGMNY;

				otl_stream stream_update_2re(1, SQL_UPDATE_SAP_AUDIT_BYREGION, db);
				stream_update_2re<< int_V_NEW_GROUP_ID<< long_V_NEWMNY<< long_V_NEWMNY
												<< long_V_NEWMNY<< str_V_ExtractSeq<< str_V_REGION
												<< int_CUR_SAP_Revenue_Code_ID;

				int_cnt_upd++;
			}
			else
			{
				otl_stream stream_insert_2re(1, SQL_INSERT_SAP_AUDIT_BYREGION, db);	
				stream_insert_2re<<int_V_NEW_GROUP_ID<< int_CUR_SAP_Revenue_Code_ID
											<< str_V_REGION<< long_V_NEWMNY<< long_V_NEWMNY
											<< long_V_NEWMNY<< str_V_ExtractSeq;

				int_cnt_inst++;
				
			}						
		}// while
				sprintf(str_msglog,"Finish INSERT %d lines and UPDATE %d lines money into table CC_TBL_DAT_SAP_AUDIT_BYREGION",int_cnt_inst,int_cnt_upd);
				bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"Case 2 RE SQL19_2",str_msglog);
	 }
	}catch(otl_exception& p){ // intercept OTL exceptions

		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.msg);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.stm_text);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.var_info);

		sprintf(str_msglog,"SQL ERROR [%s]",(char*)p.stm_text);
		bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"Case 2 RE",str_msglog);
		
		return(BL_SAP_POSTBG_NOTOK);
	} // end catch

	return(BL_SAP_POSTBG_OK);
}
// END CASE REGION_CRITERIA = '2' AND REVENUE_TYPE = 'RE'
/************************************************************************
 * Function: bl_sap_postbg_case_3RE
 * Purpose:  CASE REGION_CRITERIA = '3' AND REVENUE_TYPE = 'RE'
 * Input:    None
 * Output:	 Int : BL_SAP_POSTBG_OK
				   BL_SAP_POSTBG_NOTOK
 * Version/Programmer/Remarks:
 ************************************************************************/
int bl_sap_postbg_case_3RE()
{
	try{
		long long_Sum_REVMNY	= 0;		
		char str_REGIONCODE[41] = "";
		char str_V_REGION[11]	= "";
		int int_cnt_upd=0;
		int int_cnt_inst=0;

		sprintf(str_msglog,"CASE REGION_CRITERIA = 3 AND REVENUE_TYPE = RE");
		bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"Case 3 RE",str_msglog);

		if( strcmp(str_V_BillType,"TERMINATION") == 0 )
		{
			int_V_ID1 = 4;
			int_V_ID2 = 7;
		}
		else
		{
			int_V_ID1 = 1;
		}

		// prepare SQL 20
	/*	sprintf(str_sqlstmt,SQL_SELECT_20, int_V_ID1, int_V_ID2, int_V_GROUP_ID, str_V_VatDate, str_V_ICOName, int_CUR_SAP_Revenue_Code_ID);
		// SQL 20
		otl_stream stream_select_inv_list(1024, SQL_SELECT_20, db);
		stream_select_inv_list<< int_V_ID1<< int_V_ID2<< int_V_GROUP_ID<< str_V_VatDate
											<< str_V_ICOName<< int_CUR_SAP_Revenue_Code_ID;
*///vat_date,sap_ext_group_id,invoicing_co_name,revenue_code_id,bill_type_id,bill_type_id,revenue_code_id	
		sprintf(str_sqlstmt,SQL_SELECT_20,str_V_VatDate,int_V_GROUP_ID,str_V_ICOName,int_CUR_SAP_Revenue_Code_ID,int_V_ID1, int_V_ID2,int_CUR_SAP_Revenue_Code_ID);
		otl_stream stream_select_inv_list(1024, SQL_SELECT_20, db);
		stream_select_inv_list<<str_V_VatDate<<int_V_GROUP_ID<<str_V_ICOName
										<<int_CUR_SAP_Revenue_Code_ID<<int_V_ID1<< int_V_ID2
										<<int_CUR_SAP_Revenue_Code_ID;
		
		if( stream_select_inv_list.eof() )
		{
			sprintf(str_msglog, ERR_MSG_17,str_sqlstmt);
			bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"Case 3 RE SQL 20 [17]",str_msglog);

			int_Status = BL_SAP_POSTBG_NOTOK;
			return(int_Status);
		}

		while(!stream_select_inv_list.eof())
		{
			stream_select_inv_list >> long_Sum_REVMNY >> str_REGIONCODE;

			// prepare SQL 21
			sprintf(str_sqlstmt,SQL_SELECT_UNIQUE_SAP_REGION_MAP, str_REGIONCODE, int_V_ICOID);

			// SQL 21
			otl_stream stream_select_uni_regid(1024, SQL_SELECT_UNIQUE_SAP_REGION_MAP, db);
			stream_select_uni_regid << str_REGIONCODE<< int_V_ICOID;
			
			if(!stream_select_uni_regid.eof())
			{
				stream_select_uni_regid >> str_V_REGION;
			}
			else
			{
				sprintf(str_msglog, ERR_MSG_18,str_sqlstmt);
				bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"Case 3 RE SQL 21 [18]",str_msglog);

				int_Status = BL_SAP_POSTBG_NOTOK;
				return(int_Status);
			}

			// prepare SQL 22
			//sprintf(str_sqlstmt,SQL_SELECT_22, str_V_ExtractSeq, str_V_REGION, int_CUR_SAP_Revenue_Code_ID);

			// SQL 22
			otl_stream stream_select_3re(1024, SQL_SELECT_ORIGMNY_SAP_AUDIT_BYREGION, db);
			stream_select_3re <<str_V_ExtractSeq<< str_V_REGION<< int_CUR_SAP_Revenue_Code_ID;

			if(!stream_select_3re.eof())
			{
				// Found Update
				long long_V_ORIGMNY = 0;

				stream_select_3re >> long_V_ORIGMNY;
				long_Sum_REVMNY = long_Sum_REVMNY + long_V_ORIGMNY;

				otl_stream stream_update_3re(1, SQL_UPDATE_SAP_AUDIT_BYREGION, db);
				stream_update_3re<<int_V_NEW_GROUP_ID<< long_Sum_REVMNY<< long_Sum_REVMNY
											<< long_Sum_REVMNY<< str_V_ExtractSeq<< str_V_REGION
											<< int_CUR_SAP_Revenue_Code_ID;

				int_cnt_upd++;
			}
			else
			{
				//sprintf(str_sqlstmt,SQL_INSERT_23_2, str_V_NEW_GROUP_ID, int_CUR_SAP_Revenue_Code_ID, str_V_REGION, long_Sum_REVMNY, long_Sum_REVMNY, long_Sum_REVMNY, str_V_ExtractSeq);
				//otl_stream stream_insert_3re(1, str_sqlstmt, db);

				otl_stream stream_insert_3re(1, SQL_INSERT_SAP_AUDIT_BYREGION, db);
				stream_insert_3re<< int_V_NEW_GROUP_ID<< int_CUR_SAP_Revenue_Code_ID
											<< str_V_REGION<< long_Sum_REVMNY<< long_Sum_REVMNY
											<< long_Sum_REVMNY<< str_V_ExtractSeq;

				int_cnt_inst++;
			}											
		} // End while
		sprintf(str_msglog,"Finish INSERT %d lines and UPDATE %d lines money into table CC_TBL_DAT_SAP_AUDIT_BYREGION",int_cnt_inst,int_cnt_upd);
		bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"Case 3 RE SQL19_2",str_msglog);
	}catch(otl_exception& p){ // intercept OTL exceptions

		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.msg);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.stm_text);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.var_info);

		sprintf(str_msglog,"SQL ERROR [%s]",(char*)p.stm_text);
		bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"Case 3 RE",str_msglog);
		
		return(BL_SAP_POSTBG_NOTOK);
	} // end catch

	return(BL_SAP_POSTBG_OK);

}
// END CASE REGION_CRITERIA = '3' AND REVENUE_TYPE = 'RE'
/************************************************************************
 * Function: bl_sap_postbg_case_4RE
 * Purpose:  CASE REGION_CRITERIA = '4' AND REVENUE_TYPE = 'RE'
 * Input:    None
 * Output:	 Int : BL_SAP_POSTBG_OK
				   BL_SAP_POSTBG_NOTOK
 * Version/Programmer/Remarks:
 ************************************************************************/
int bl_sap_postbg_case_4RE()
{
	try{
		char str_V_REGION[11]	 =  "00";
		long long_V_NEWMNY	= long_CUR_SAP_Orig_Revenue_MNY;

		sprintf(str_msglog,"CASE REGION_CRITERIA = 4 AND REVENUE_TYPE = RE");
		bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"Case 4 RE",str_msglog);

		// prepare SQL 24
		//sprintf(str_sqlstmt,SQL_SELECT_24, str_V_ExtractSeq, str_V_REGION, int_CUR_SAP_Revenue_Code_ID);

		otl_stream stream_select_4re(1024, SQL_SELECT_ORIGMNY_SAP_AUDIT_BYREGION, db);
		stream_select_4re<<str_V_ExtractSeq<< str_V_REGION<< int_CUR_SAP_Revenue_Code_ID;

		if(!stream_select_4re.eof())
		{
			// Found Update
			long long_V_ORIGMNY = 0;

			stream_select_4re >> long_V_ORIGMNY;
			long_V_NEWMNY = long_V_NEWMNY + long_V_ORIGMNY;

			otl_stream stream_update_4re(1, SQL_UPDATE_SAP_AUDIT_BYREGION, db);
			stream_update_4re << int_V_NEW_GROUP_ID<< long_V_NEWMNY<< long_V_NEWMNY
										<< long_V_NEWMNY<< str_V_ExtractSeq<< str_V_REGION
										<< int_CUR_SAP_Revenue_Code_ID;
		
			sprintf(str_msglog,"Update money in table CC_TBL_DAT_SAP_AUDIT_BYREGION");
			bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"Case 4 RE SQL25_1",str_msglog);
		}
		else
		{

			//sprintf(str_sqlstmt,SQL_INSERT_25_2,str_V_NEW_GROUP_ID, int_CUR_SAP_Revenue_Code_ID, str_V_REGION, long_V_NEWMNY, long_V_NEWMNY, long_V_NEWMNY, str_V_ExtractSeq);
			//otl_stream stream_insert_4re(1, str_sqlstmt, db);
			// SQL 25.2
			otl_stream stream_insert_4re(1, SQL_INSERT_SAP_AUDIT_BYREGION, db);
			stream_insert_4re<< int_V_NEW_GROUP_ID<< int_CUR_SAP_Revenue_Code_ID
										<< str_V_REGION<< long_V_NEWMNY<< long_V_NEWMNY<< long_V_NEWMNY
										<< str_V_ExtractSeq;
				
			sprintf(str_msglog,"Insert money into table CC_TBL_DAT_SAP_AUDIT_BYREGION");
			bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"Case 4 RE SQL25_2",str_msglog);
		}
	}catch(otl_exception& p){ // intercept OTL exceptions

		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.msg);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.stm_text);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.var_info);

		sprintf(str_msglog,"SQL ERROR [%s]",(char*)p.stm_text);
		bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"Case 4 RE",str_msglog);
		
		return(BL_SAP_POSTBG_NOTOK);
	} // end catch

	return(BL_SAP_POSTBG_OK);
}
// END CASE REGION_CRITERIA = '4' AND REVENUE_TYPE = 'RE'
/************************************************************************
 * Function: bl_sap_postbg_case_MORE5RE
 * Purpose:  CASE REGION_CRITERIA > '5' AND REVENUE_TYPE = 'RE'
 * Input:    None
 * Output:	 Int : BL_SAP_POSTBG_OK
				   BL_SAP_POSTBG_NOTOK
 * Add By :	Arjaree(Supannika) Mathong
 *	Date :		15 Aug 2005																
 ************************************************************************/
int bl_sap_postbg_case_MORE5RE()
{
	try{
		char str_V_REGION[11]	 =  "00";
		long long_V_NEWMNY	= long_CUR_SAP_Orig_Revenue_MNY;

		sprintf(str_msglog,"CASE REGION_CRITERIA > 5 AND REVENUE_TYPE = RE");
		bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"Case MORE 5 RE",str_msglog);

		// SQL 42
		//sprintf(str_sqlstmt,SQL_SELECT_42,int_V_ICOID, int_CUR_SAP_Region_Criteria,str_CUR_SAP_Revenue_Type );
		otl_stream stream_select_regid(1024, SQL_SELECT_42, db);
		stream_select_regid<<int_V_ICOID<< int_CUR_SAP_Region_Criteria<< str_CUR_SAP_Revenue_Type;
		
		str_V_REGION[0] ='\0';
		
		if(!stream_select_regid.eof())
		{
				stream_select_regid >> str_V_REGION;
		}else{
				strcpy(str_V_REGION,"0");
		}
	
		// SQL 43
		//sprintf(str_sqlstmt,SQL_SELECT_43, str_V_ExtractSeq, str_V_REGION, int_CUR_SAP_Revenue_Code_ID);
		otl_stream stream_select_more5re(1024, SQL_SELECT_43, db);
		stream_select_more5re<<str_V_ExtractSeq<< str_V_REGION<< int_CUR_SAP_Revenue_Code_ID;

		if(!stream_select_more5re.eof())
		{
			// Found Update
			long long_V_ORIGMNY = 0;

			stream_select_more5re >> long_V_ORIGMNY;
			long_V_NEWMNY = long_V_NEWMNY + long_V_ORIGMNY;

			otl_stream stream_update_more5re(1, SQL_UPDATE_SAP_AUDIT_BYREGION, db);
			stream_update_more5re<<int_V_NEW_GROUP_ID<< long_V_NEWMNY<< long_V_NEWMNY
													<< long_V_NEWMNY<< str_V_ExtractSeq<< str_V_REGION
													<< int_CUR_SAP_Revenue_Code_ID;

			sprintf(str_msglog,"Update money in table CC_TBL_DAT_SAP_AUDIT_BYREGION");
			bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"Case MORE 5 RE SQL44_1",str_msglog);
		}
		else
		{
			otl_stream stream_insert_more5re(1, SQL_INSERT_SAP_AUDIT_BYREGION, db);
			stream_insert_more5re<<int_V_NEW_GROUP_ID<< int_CUR_SAP_Revenue_Code_ID
												<< str_V_REGION<< long_V_NEWMNY<< long_V_NEWMNY
												<< long_V_NEWMNY << str_V_ExtractSeq;

			sprintf(str_msglog,"Insert money into table CC_TBL_DAT_SAP_AUDIT_BYREGION");
			bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"Case MORE 5 RE SQL44_2",str_msglog);
		}
	}catch(otl_exception& p){ // intercept OTL exceptions

		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.msg);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.stm_text);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.var_info);

		sprintf(str_msglog,"SQL ERROR [%s]",(char*)p.stm_text);
		bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"Case MORE 5 RE",str_msglog);
		
		return(BL_SAP_POSTBG_NOTOK);
	} // end catch

	return(BL_SAP_POSTBG_OK);
}
// END CASE REGION_CRITERIA > '5' AND REVENUE_TYPE = 'RE'
/************************************************************************
 * Function: bl_sap_postbg_case_3DS
 * Purpose:  CASE REGION_CRITERIA = '3' AND REVENUE_TYPE = 'DS'
 * Input:    None
 * Output:	 Int : BL_SAP_POSTBG_OK
				   BL_SAP_POSTBG_NOTOK
 * Version/Programmer/Remarks:
 ************************************************************************/
int bl_sap_postbg_case_3DS()
{
	try{
		int int_V_CHARGERCID = 0;
		char str_REGIONCODE[11] = "";
		char str_V_REGION[11] =  "";
		long long_Tot_Revenue = 0;

		sprintf(str_msglog,"CASE REGION_CRITERIA = 3 AND REVENUE_TYPE = DS");
		bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"Case 3 DS",str_msglog);

		if( strcmp(str_V_BillType,"TERMINATION") == 0 )
		{
			int_V_ID1 = 4;
			int_V_ID2 = 7;
		}
		else
		{
			int_V_ID1 = 1;
			int_V_ID2 = 1;
		}

		// prepare SQL 26
	/*	sprintf(str_sqlstmt,SQL_SELECT_26, int_V_ID1, int_V_ID2, int_V_GROUP_ID, str_V_VatDate, str_V_ICOName, int_CUR_SAP_Revenue_Code_ID);

		// SQL 26
		otl_stream stream_select_inv_revlst(1024, SQL_SELECT_26, db);
		stream_select_inv_revlst<<int_V_ID1<< int_V_ID2<< int_V_GROUP_ID<< str_V_VatDate<< str_V_ICOName
											<< int_CUR_SAP_Revenue_Code_ID;
*/// 26 == 20
		sprintf(str_sqlstmt,SQL_SELECT_20,str_V_VatDate,int_V_GROUP_ID,str_V_ICOName,int_CUR_SAP_Revenue_Code_ID,int_V_ID1, int_V_ID2,int_CUR_SAP_Revenue_Code_ID);
		otl_stream stream_select_inv_revlst(1024, SQL_SELECT_20, db);
		stream_select_inv_revlst<<str_V_VatDate<<int_V_GROUP_ID<<str_V_ICOName
										<<int_CUR_SAP_Revenue_Code_ID<<int_V_ID1<< int_V_ID2
										<<int_CUR_SAP_Revenue_Code_ID;
										
		if( stream_select_inv_revlst.eof() )
		{
			sprintf(str_msglog, ERR_MSG_19, str_sqlstmt);
			bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"Case 3 DS SQL 26 [19]",str_msglog);

			int_Status = BL_SAP_POSTBG_NOTOK;
			return(int_Status);
		}

		while(!stream_select_inv_revlst.eof())
		{
			stream_select_inv_revlst >> long_Tot_Revenue >> str_REGIONCODE;

			// prepare SQL 27
			sprintf(str_sqlstmt,SQL_SELECT_UNIQUE_SAP_REGION_MAP, str_REGIONCODE, int_V_ICOID);
			
			otl_stream stream_select_uni_sapid(1024, SQL_SELECT_UNIQUE_SAP_REGION_MAP, db);
			stream_select_uni_sapid<< str_REGIONCODE<< int_V_ICOID;
			
			if(!stream_select_uni_sapid.eof())
			{
				stream_select_uni_sapid >> str_V_REGION;
			}
			else
			{
				sprintf(str_msglog, ERR_MSG_20, str_sqlstmt);
				bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"Case 3 DS SQL 27 [20]",str_msglog);

				int_Status = BL_SAP_POSTBG_NOTOK;
				return(int_Status);
			}

			// prepare SQL 27N
			sprintf(str_sqlstmt,SQL_SELECT_27N, int_CUR_SAP_Revenue_Code_ID, int_V_ICOID);

			otl_stream stream_select_chargeid(1024, SQL_SELECT_27N, db);
			stream_select_chargeid<<int_CUR_SAP_Revenue_Code_ID<< int_V_ICOID;

			if(!stream_select_chargeid.eof())
			{
				stream_select_chargeid >> int_V_CHARGERCID;
			}
			else
			{
				sprintf(str_msglog, ERR_MSG_21, str_sqlstmt);
				bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"Case 3 DS SQL 27N [21]",str_msglog);

				int_Status = BL_SAP_POSTBG_NOTOK;
				return(int_Status);					
			}

			// prepare SQL 28
			//sprintf(str_sqlstmt,SQL_SELECT_28, str_V_ExtractSeq, str_V_REGION, int_V_CHARGERCID);
			otl_stream stream_select_postds(1024, SQL_SELECT_28, db);
			stream_select_postds<<str_V_ExtractSeq<< str_V_REGION<< int_V_CHARGERCID;

			if(!stream_select_postds.eof())
			{
				// Found Update
				long long_V_PDISCOUNT = 0;

				stream_select_postds >> long_V_PDISCOUNT;
				long_V_PDISCOUNT = long_V_PDISCOUNT + long_Tot_Revenue;

				if( long_V_PDISCOUNT < 0 )
					{
						// Error Discount more than original money
						sprintf(str_msglog,"Discount more than original Money [%ld] for Extraction Seq [%s] and Revenue Code ID [%d]", long_Tot_Revenue, str_V_ExtractSeq, int_CUR_SAP_Revenue_Code_ID);
						bl_Log_Print(file_logfp,BLSAPPOSTBG_WARNING_LOG,"Case 3 DS",str_msglog);

					}

				otl_stream stream_update_revreg(1, SQL_UPDATE_29_1, db);
				stream_update_revreg<<int_V_NEW_GROUP_ID<< long_V_PDISCOUNT
													<< long_V_PDISCOUNT<< str_V_ExtractSeq<< str_V_REGION
													<< int_V_CHARGERCID;

				sprintf(str_msglog,"Update money in table CC_TBL_DAT_SAP_AUDIT_BYREGION");
				bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"Case 3 DS SQL29_1",str_msglog);
			}
			else
			{
				// Not found
				// output to file
				sprintf(str_msglog,"ERROR: REVENUE CODE %s DOES NOT HAVE ANY REVENUE. DISCOUNT %d CANNOT OFFSET THE REVENUE",str_CUR_SAP_Revenue_Code_Name, int_CUR_SAP_Revenue_Code_ID);
				bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG, "Case 3 DS", str_msglog);
			}											
					
			otl_stream stream_insert_rev_disc(1, SQL_INSERT_SAP_AUDIT_REV_DISC, db);
			stream_insert_rev_disc<< int_V_NEW_GROUP_ID<< str_V_ExtractSeq<< int_CUR_SAP_Revenue_Code_ID
												<< int_V_CHARGERCID<< long_Tot_Revenue<< "DS" <<str_V_REGION;

			sprintf(str_msglog,"Insert record into table CC_TBL_DAT_SAP_AUDIT_REV_DISC");
			bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"Case 3 DS SQL29_2",str_msglog);
		}

	}catch(otl_exception& p){ // intercept OTL exceptions

		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.msg);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.stm_text);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.var_info);

		sprintf(str_msglog,"SQL ERROR [%s]",(char*)p.stm_text);
		bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"Case 3 DS",str_msglog);

		return(BL_SAP_POSTBG_NOTOK);
	} // end catch

	return(BL_SAP_POSTBG_OK);
}
// END CASE REGION_CRITERIA = '3' AND REVENUE_TYPE = 'DS'
/************************************************************************
 * Function: bl_sap_postbg_case_5DSDG
 * Purpose:  CASE REGION_CRITERIA = '5' AND REVENUE_TYPE = 'DS' or 'DG'
 * Input:    None
 * Output:	 Int : BL_SAP_POSTBG_OK
				   BL_SAP_POSTBG_NOTOK
 * Version/Programmer/Remarks:
 ************************************************************************/
int bl_sap_postbg_case_5DSDG()
{
	try{
		int  int_TMP_SAP_Ext_Group_ID	= 0;
		long long_V_TOTALPSMNY			= 0;
		long long_V_TOTALPGMNY			= 0;
		long long_V_ORIGREVMNY			= 0;
		long long_V_ORIGPSDISCOUNT		= 0;
		long long_V_ORIGPGDISCOUNT		= 0;
		long long_V_EACHDISCMNY			= 0;
		long long_V_TOTALMNY			= 0;
		long long_V_DERIVEDDISCMNY		= 0;		
		int  int_V_CHARGERCID			= 0;
		char str_V_REGION[11]			= "";
		char str_TMP_ExtractSeq[11]		= "";
		char str_dat_disc_adj[100]		= "";
		char str_rev1[100]				= "";
		char str_rev2[100]				= "";

		long long_TMP_MNY = 0;
		double double_TMP_MNY = 0;

		sprintf(str_msglog,"CASE REGION_CRITERIA = 5 AND REVENUE_TYPE = DS or DG");
		bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"Case 5 DSDG",str_msglog);

		// prepare SQL 30
		//sprintf(str_sqlstmt,SQL_SELECT_30, str_V_ExtractSeq, int_CUR_SAP_Revenue_Code_ID, int_V_ICOID);

		otl_stream stream_select_discount(1024, SQL_SELECT_30, db);
		stream_select_discount<<str_V_ExtractSeq<< int_CUR_SAP_Revenue_Code_ID<< int_V_ICOID;
				
		if(!stream_select_discount.eof())
		{
			stream_select_discount >> long_V_TOTALPSMNY >> long_V_TOTALPGMNY;
		}
		// To prevent error from revenue adjustment 152 and 431
		// Special handling for CR 429
		// Modify on Dec 16


		sprintf(str_sqlstmt,SQL_SELECT_30_1);

		otl_stream stream_sel_dat_disc_adj(1024, SQL_SELECT_30_1, db);
		if(!stream_sel_dat_disc_adj.eof())
		{
			stream_sel_dat_disc_adj >> str_dat_disc_adj;
			sprintf(str_msglog,"Discount Adjustment Revenue [%s]", str_dat_disc_adj);
			bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"Case 5 DSDG SQL33_1",str_msglog);
		}
		else
		{
			sprintf(str_msglog, ERR_MSG_26, str_sqlstmt);
			bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"Case 5 DSDG SQL 30_1 [26]",str_msglog);

			int_Status = BL_SAP_POSTBG_NOTOK;
			return(int_Status);		
		}

		if( SplitRevenue(str_dat_disc_adj, str_rev1, str_rev2) == BL_SAP_POSTBG_NOTOK ) 
		{
			sprintf(str_msglog, ERR_MSG_27, str_dat_disc_adj);
			bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"Split Discount Adjustment Revenue [27]",str_msglog);

			int_Status = BL_SAP_POSTBG_NOTOK;
			return(int_Status);		
		}

		// Check whether revenue is equal to discount adjustment
		if( atoi(str_rev1) == int_CUR_SAP_Revenue_Code_ID )
		{
			// prepare SQL 30_2
			//sprintf(str_sqlstmt,SQL_SELECT_30_2, str_V_ExtractSeq, str_rev2);
			otl_stream stream_sel_chk_disc_adj_rev_to(1024, SQL_SELECT_30_2, db);
			stream_sel_chk_disc_adj_rev_to<<str_V_ExtractSeq<< atoi(str_rev2);

			if(stream_sel_chk_disc_adj_rev_to.eof())
			{
				// comment out by bas on Dec 19, 2003
				// Change base discount allocation to 1 instead of null value
				//long_V_TOTALPSMNY = 1;
				//long_V_TOTALPGMNY = 1;

				// insert revenue GSM voice of BKK (431)

				// prepare SQL 30_3
			//	sprintf(str_sqlstmt,SQL_INSERT_30_3, int_V_GROUP_ID, str_rev2, str_V_ExtractSeq);
			//	otl_stream stream_sel_chk_disc_adj_rev_to( 1, str_sqlstmt, db);
				// SQL 30_3
				otl_stream stream_sel_chk_disc_adj_rev_to( 1, SQL_INSERT_30_3, db);
				stream_sel_chk_disc_adj_rev_to<< int_V_GROUP_ID<< atoi(str_rev2) << str_V_ExtractSeq;

				sprintf(str_msglog,"Insert revenue 431 region 01 for discount adjustment 152");
				bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"5 DSDG",str_msglog);
			}
		}
	
		// end special handling CR429

		// prepare SQL 31
		sprintf(str_sqlstmt,SQL_SELECT_31, str_V_ExtractSeq, int_CUR_SAP_Revenue_Code_ID, int_V_ICOID);

		otl_stream stream_select_all_dis(1024, SQL_SELECT_31, db);
		stream_select_all_dis<<str_V_ExtractSeq<< int_CUR_SAP_Revenue_Code_ID<< int_V_ICOID;

		if( stream_select_all_dis.eof() )
		{
			sprintf(str_msglog, ERR_MSG_23, str_sqlstmt);
			bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"Case 5 DSDG SQL 31 [23]",str_msglog);

			int_Status = BL_SAP_POSTBG_NOTOK;
			return(int_Status);		
		}

		while(!stream_select_all_dis.eof())
		{
			stream_select_all_dis >> int_TMP_SAP_Ext_Group_ID >> int_V_CHARGERCID >> str_V_REGION >> long_V_ORIGREVMNY >> long_V_ORIGPSDISCOUNT >> long_V_ORIGPGDISCOUNT >> str_TMP_ExtractSeq;

			if( strcmp(str_CUR_SAP_Revenue_Type,"DS") == 0 )
			{
				long_V_EACHDISCMNY = long_V_ORIGPSDISCOUNT;
				long_V_TOTALMNY = long_V_TOTALPSMNY;
			}
			else if( strcmp(str_CUR_SAP_Revenue_Type,"DG") == 0 )
			{
				long_V_EACHDISCMNY = long_V_ORIGPGDISCOUNT;
				long_V_TOTALMNY = long_V_TOTALPGMNY;		
			}

			// Special handling for revenue 152
			if( atoi(str_rev1) == int_CUR_SAP_Revenue_Code_ID )
			{
				//long_V_EACHDISCMNY = 0;
				long_V_DERIVEDDISCMNY = long_CUR_SAP_Orig_Revenue_MNY;
			}
			else
			{
				if( (strcmp(str_CUR_SAP_Revenue_Type,"DS") == 0) && (long_V_ORIGPSDISCOUNT < 0) )
				{
					sprintf(str_msglog,"Post Specific money is negative and will ignore discount allocation");
					bl_Log_Print(file_logfp,BLSAPPOSTBG_WARNING_LOG,"Case 5 DSDG",str_msglog);

					continue;
				}
				else if( (strcmp(str_CUR_SAP_Revenue_Type,"DS") == 0) && (long_V_ORIGPGDISCOUNT < 0) )
				{
					sprintf(str_msglog,"Post General money is negative and will ignore discount allocation");
					bl_Log_Print(file_logfp,BLSAPPOSTBG_WARNING_LOG,"Case 5 DSDG",str_msglog);

					continue;
				}

				// ROUND UP
				double_TMP_MNY = (double)long_V_EACHDISCMNY / (double)long_V_TOTALMNY;
				double_TMP_MNY = double_TMP_MNY * (double)long_CUR_SAP_Orig_Revenue_MNY;
				double_TMP_MNY = double_TMP_MNY / 1000;
				double_TMP_MNY = RoundUp2(double_TMP_MNY);
				long_V_DERIVEDDISCMNY = (long)(double_TMP_MNY * 1000);
			}

			// prepare SQL 32
			//sprintf(str_sqlstmt,SQL_SELECT_32, str_V_ExtractSeq, str_V_REGION, int_V_CHARGERCID );

			otl_stream stream_select_5dsdg(1024, SQL_SELECT_32, db);
			stream_select_5dsdg<<str_V_ExtractSeq<< str_V_REGION<< int_V_CHARGERCID;

			if(!stream_select_5dsdg.eof())
			{
				if( strcmp(str_CUR_SAP_Revenue_Type,"DS") == 0 )
				{
					long_TMP_MNY = long_V_EACHDISCMNY + long_V_DERIVEDDISCMNY;

					if( long_TMP_MNY < 0 )
					{
						// Error Discount more than original money
						sprintf(str_msglog,"Discount [%ld] more than original Money [%ld] for Extraction Seq [%s] and Revenue Code ID [%d]", long_V_EACHDISCMNY, long_V_DERIVEDDISCMNY, str_V_ExtractSeq, int_CUR_SAP_Revenue_Code_ID);
						bl_Log_Print(file_logfp,BLSAPPOSTBG_WARNING_LOG,"Case 5 DSDG",str_msglog);

//						int_Status = BL_SAP_POSTBG_NOTOK;
//						return(int_Status);
					}

					// prepare SQL 331
					//sprintf(str_sqlstmt,SQL_UPDATE_33_1, int_V_NEW_GROUP_ID, long_TMP_MNY, long_TMP_MNY, str_V_ExtractSeq, str_V_REGION, int_V_CHARGERCID );
					//saowanir : 22/05/2007 : int_V_NEW_GROUP_ID to str_V_NEW_GROUP_ID
					//sprintf(str_sqlstmt,SQL_UPDATE_33_1, str_V_NEW_GROUP_ID, long_TMP_MNY, long_TMP_MNY, str_V_ExtractSeq, str_V_REGION, int_V_CHARGERCID );

					// SQL 331
					otl_stream stream_update_5ds(1, SQL_UPDATE_33_1, db);
					stream_update_5ds<<int_V_NEW_GROUP_ID<< long_TMP_MNY<< long_TMP_MNY
													<< str_V_ExtractSeq<< str_V_REGION<< int_V_CHARGERCID;

					sprintf(str_msglog,"Update money in table CC_TBL_DAT_SAP_AUDIT_BYREGION");
					bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"Case 5 DSDG SQL33_1",str_msglog);
				}
				else if( strcmp(str_CUR_SAP_Revenue_Type,"DG") == 0 )
				{
					long_TMP_MNY = long_V_EACHDISCMNY + long_V_DERIVEDDISCMNY;

					if( long_TMP_MNY < 0 )
					{
						// Error Discount more than original money
						sprintf(str_msglog,"Discount [%ld] more than original Money [%ld] for Extraction Seq [%s] and Revenue Code ID [%d]", long_V_EACHDISCMNY, long_V_DERIVEDDISCMNY, str_V_ExtractSeq, int_CUR_SAP_Revenue_Code_ID);
						bl_Log_Print(file_logfp,BLSAPPOSTBG_WARNING_LOG,"Case 5 DSDG",str_msglog);

//						int_Status = BL_SAP_POSTBG_NOTOK;
//						return(int_Status);
					}

					// prepare SQL 332
					//sprintf(str_sqlstmt,SQL_UPDATE_33_2, int_V_NEW_GROUP_ID, long_TMP_MNY, str_V_ExtractSeq, str_V_REGION, int_V_CHARGERCID );
					//saowanir : 22/05/2007 : int_V_NEW_GROUP_ID to str_V_NEW_GROUP_ID
					//sprintf(str_sqlstmt,SQL_UPDATE_33_2, str_V_NEW_GROUP_ID, long_TMP_MNY, str_V_ExtractSeq, str_V_REGION, int_V_CHARGERCID );

					// SQL 332
					otl_stream stream_update_5dg(1, SQL_UPDATE_33_2, db);
					stream_update_5dg<< int_V_NEW_GROUP_ID<< long_TMP_MNY<< str_V_ExtractSeq<< str_V_REGION<< int_V_CHARGERCID;
			
					sprintf(str_msglog,"Insert money into table CC_TBL_DAT_SAP_AUDIT_BYREGION");
					bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"Case 5 DSDG SQL33_2",str_msglog);
				}
			}
			else
			{
				// Not found report error
				sprintf(str_msglog,"ERROR: REVENUE CODE [%s] DOES NOT HAVE ANY REVENUE. DISCOUNT [%d] CANNOT OFFSET THE REVENUE", str_CUR_SAP_Revenue_Code_Name, int_CUR_SAP_Revenue_Code_ID);
				bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"Case 5 DSDG SQL 32",str_msglog);

				int_Status = BL_SAP_POSTBG_NOTOK;
				return(int_Status);
			}

			// prepare SQL 333
			//sprintf(str_sqlstmt,SQL_INSERT_33_3, int_V_NEW_GROUP_ID, str_V_ExtractSeq, int_CUR_SAP_Revenue_Code_ID, int_V_CHARGERCID, long_V_DERIVEDDISCMNY, str_CUR_SAP_Revenue_Type, str_V_REGION);
			//saowanir : 22/05/2007 : int_V_NEW_GROUP_ID to str_V_NEW_GROUP_ID
			//sprintf(str_sqlstmt,SQL_INSERT_33_3, str_V_NEW_GROUP_ID, str_V_ExtractSeq, int_CUR_SAP_Revenue_Code_ID, int_V_CHARGERCID, long_V_DERIVEDDISCMNY, str_CUR_SAP_Revenue_Type, str_V_REGION);
			//otl_stream stream_insert_5dsdg(1, str_sqlstmt, db);
			// SQL 333
			otl_stream stream_insert_5dsdg(1, SQL_INSERT_SAP_AUDIT_REV_DISC, db);
			stream_insert_5dsdg<<int_V_NEW_GROUP_ID<< str_V_ExtractSeq<< int_CUR_SAP_Revenue_Code_ID
											<< int_V_CHARGERCID<< long_V_DERIVEDDISCMNY<< str_CUR_SAP_Revenue_Type
											<< str_V_REGION;
	
			sprintf(str_msglog,"Insert discount into table CC_TBL_DAT_SAP_AUDIT_REV_DISC");
			bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"Case 5 DSDG SQL33_3",str_msglog);
		}//while
	}catch(otl_exception& p){ // intercept OTL exceptions

		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.msg);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.stm_text);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.var_info);

		sprintf(str_msglog,"SQL ERROR [%s]",(char*)p.stm_text);
		bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"Case 5 DSDG'",str_msglog);

		return(BL_SAP_POSTBG_NOTOK);
	} // end catch

	return(BL_SAP_POSTBG_OK);

}
// END CASE REGION_CRITERIA = '5' AND REVENUE_TYPE = 'DS'  OR 'DG'
/************************************************************************
 * Function: bl_sap_postbg_check_diff_mny
 * Purpose:  CHECK DIFFERENT MONEY AFTER ROUND TO NEAREST VALUE 
 *			 FOR EACH EXTRACTION SEQUENCE
 * Input:    None
 * Output:	 Int : BL_SAP_POSTBG_OK
				   BL_SAP_POSTBG_NOTOK
 * Version/Programmer/Remarks:
 ************************************************************************/
int bl_sap_postbg_check_diff_mny()
{
	long long_V_TOTALDERIVEDREVENUE = 0;
	long long_V_REVCODEVALUE = 0;
	long long_V_REGIONVALUE = 0;
	long long_TMP_PSDISMNY = 0;
	long long_TMP_PGDISMNY = 0;
	long long_ORIG_REVENUE_MNY = 0;
	
	char str_V_REVCODE[41] = "";
	char str_V_REGION[41] = "";

	try{
		// prepare SQL 34
		//sprintf(str_sqlstmt,SQL_SELECT_34,str_V_ExtractSeq);

		// SQL 34
		otl_stream stream_select_totderev(1024, SQL_SELECT_34, db);
		stream_select_totderev << str_V_ExtractSeq;

		if(!stream_select_totderev.eof())
		{
			stream_select_totderev >> long_V_TOTALDERIVEDREVENUE;

			if( long_V_TOTALDERIVEDREVENUE != long_V_TotalRevenue )
			{
				// OUTPUT TO FIE
				sprintf(str_msglog,"LOG: DERIVED REVENUE: %ld IS DIFFERENT FROM ORIGINAL REVENUE %ld",long_V_TOTALDERIVEDREVENUE,long_V_TotalRevenue);
				bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG, "DIFF MNY", str_msglog);

				if( int_V_ICOID == 2 )
				{
					strcpy(str_V_REVCODE,"AIS_INV_DEFAULT REVCODE");
					strcpy(str_V_REGION,"AIS_INV_DEFAULT REGION");
				}
				else if( int_V_ICOID == 3 ) //Changed 20130904 : Somkiet C.
				{
					strcpy(str_V_REVCODE,"DPC_INV_DEFAULT REVCODE");
					strcpy(str_V_REGION,"DPC_INV_DEFAULT REGION");
				}
				else if( int_V_ICOID == 4 ) //Add 20130904 : Somkiet C. 
				{
					strcpy(str_V_REVCODE,"AWN_INV_DEFAULT REVCODE");
					strcpy(str_V_REGION,"AWN_INV_DEFAULT REGION");
				}

				// prepare SQL 35
				//sprintf(str_sqlstmt,SQL_SELECT_35,str_V_REVCODE);

				// SQL 35
				otl_stream stream_select_revvalue(1024, SQL_SELECT_35, db);
				stream_select_revvalue<< str_V_REVCODE;
						
				if(!stream_select_revvalue.eof())
				{
					stream_select_revvalue >> long_V_REVCODEVALUE;
				}
				else
				{
					sprintf(str_msglog, ERR_MSG_24, str_V_REVCODE);
					bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"SQL 35 [24]",str_msglog);

					int_Status = BL_SAP_POSTBG_NOTOK;
					return(int_Status);
				}

//				if( long_V_REVCODEVALUE == NULL )
//				{
//					long_V_REVCODEVALUE = 0;
//				}

				// prepare SQL 36
				//sprintf(str_sqlstmt,SQL_SELECT_36,str_V_REGION);

				// SQL 36
				otl_stream stream_select_regvalue(1024, SQL_SELECT_36, db);
				stream_select_regvalue<< str_V_REGION;

				if(!stream_select_regvalue.eof())
				{
					stream_select_regvalue >> long_V_REGIONVALUE;
				}
				else
				{
					sprintf(str_msglog, ERR_MSG_25, str_V_REGION);
					bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"SQL 36 [25]",str_msglog);

					int_Status = BL_SAP_POSTBG_NOTOK;
					return(int_Status);				
				}

				char str_V_REGIONVALUE[41] = "";

				sprintf(str_V_REGIONVALUE,"%.2ld",long_V_REGIONVALUE);

				// prepare SQL 37.0
				//sprintf(str_sqlstmt,SQL_SELECT_37_0, int_V_NEW_GROUP_ID, str_V_ExtractSeq, long_V_REVCODEVALUE, str_V_REGIONVALUE);
				//sprintf(str_sqlstmt,SQL_SELECT_37_0, str_V_NEW_GROUP_ID, str_V_ExtractSeq, long_V_REVCODEVALUE, str_V_REGIONVALUE);

				// SQL 37.0
				otl_stream stream_select_dsdgmny(1024, SQL_SELECT_37_0, db);
				stream_select_dsdgmny << int_V_NEW_GROUP_ID << str_V_ExtractSeq
														<< long_V_REVCODEVALUE << str_V_REGIONVALUE;
						
				if(!stream_select_dsdgmny.eof())
				{
					stream_select_dsdgmny >> long_ORIG_REVENUE_MNY >> long_TMP_PSDISMNY >> long_TMP_PGDISMNY;
				}
				else
				{
					// My add
					long_ORIG_REVENUE_MNY	= 0;
					long_TMP_PSDISMNY		= 0;
					long_TMP_PGDISMNY		= 0;
				}

				long_ORIG_REVENUE_MNY = long_ORIG_REVENUE_MNY + (long_V_TotalRevenue - long_V_TOTALDERIVEDREVENUE );
				long_TMP_PSDISMNY = long_TMP_PSDISMNY + (long_V_TotalRevenue - long_V_TOTALDERIVEDREVENUE );
				long_TMP_PGDISMNY = long_TMP_PGDISMNY + (long_V_TotalRevenue - long_V_TOTALDERIVEDREVENUE );

				// prepare SQL 37
				//sprintf(str_sqlstmt,SQL_UPDATE_37, long_ORIG_REVENUE_MNY, long_TMP_PSDISMNY, long_TMP_PGDISMNY, int_V_NEW_GROUP_ID, str_V_ExtractSeq, long_V_REVCODEVALUE, str_V_REGIONVALUE);
				//sprintf(str_sqlstmt,SQL_UPDATE_37, long_ORIG_REVENUE_MNY, long_TMP_PSDISMNY, long_TMP_PGDISMNY, str_V_NEW_GROUP_ID, str_V_ExtractSeq, long_V_REVCODEVALUE, str_V_REGIONVALUE);

				// SQL 37
				otl_stream stream_update_sapval(1, SQL_UPDATE_37, db);
				stream_update_sapval<< long_ORIG_REVENUE_MNY<< long_TMP_PSDISMNY<< long_TMP_PGDISMNY
													<< int_V_NEW_GROUP_ID << str_V_ExtractSeq<< long_V_REVCODEVALUE
													<< str_V_REGIONVALUE;

				sprintf(str_msglog,"Insert Diff MNY into CC_TBL_DAT_SAP_AUDIT_BYREGION: at revenue code [%ld] region [%s]", long_V_REVCODEVALUE, str_V_REGIONVALUE);
				bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"SQL 37",str_msglog);
			} // End if( long_V_TOTALDERIVEDREVENUE != long_V_TotalRevenue )
		} // End SQL 34
	}catch(otl_exception& p){ // intercept OTL exceptions

		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.msg);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.stm_text);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.var_info);
		
		sprintf(str_msglog,"SQL ERROR [%s]",(char*)p.stm_text);
		bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"DIFF MONEY",str_msglog);

		return(BL_SAP_POSTBG_NOTOK);
	} // end catch

	return(BL_SAP_POSTBG_OK);

}
// END CHECK DIFFERENT MONEY AFTER ROUND TO NEAREST VALUE FOR EACH EXTRACTION SEQUENCE
/************************************************************************
 * Function: main
 * Purpose:  Main Program
 * Input:    
 * Output:   
 * Version/Programmer/Remarks: 
 * Version 2.00.01  prissana.p     modify SQL_SELECT_6  20180109
 ************************************************************************/
int main(int argc, char *argv[])
{
	// print start message
	char		str_sysdate[30];
	char		env_database[MAX_BUFFER];
	
	int int_Mode = NORMAL_MODE;
	int int_TOTInv = 0;
	char str_V_Status[11] = "";

	int_fwd_status = 0;
	 char str_version[20+1];
	 int int_runmode=0;
	 int int_count=0;
	 
		#ifdef RBM_VERSION	
		sprintf(str_version, "%s.%s.%s", RBM_VERSION, SVN_REV, "2.00.01" );
		#else	
		sprintf(str_version, "%s", "2.00.01" );
		#endif
	 
        if((argc==2) && (strcmp(argv[1],"-v")==0))
        {
           sprintf(str_msglog,"BLC0080501_SAP_POSTBG %s\n",str_version);
           printf("%s",str_msglog);
           exit(0);
        }

		if((argc==15) || (argc==16))
		{
			if( strcmp(argv[14],"N") == 0 )
			{
				int_V_GROUP_ID = 0;
				int_runmode=0;
				sprintf(str_msglog,"RUNNING MODE [NORMAL]");
				printGenevaMessage("INFORM", SAP_POSTBG_PROG_NAME, "MAIN", str_msglog);
			}
			else if( strcmp(argv[14],"E") == 0)
			{
				int_Mode = ERROR_MODE;
				int_runmode=0;
				int_V_GROUP_ID = atoi(argv[15]);
				sprintf(str_msglog,"RUNNING MODE [ERROR HANDLING] GROUP [%s]",argv[15]);
				printGenevaMessage("INFORM", SAP_POSTBG_PROG_NAME, "MAIN", str_msglog);
			}
			else
			{
					int_runmode=1;
			}
		}	
		else
		{
			int_runmode=1;
		}
	
	if(int_runmode==1)
	{
			sprintf(str_msglog,"Usage:BLC0080501_SAP_POSTBG 1 2 3 4 5 6 7 8 9 10 11 12 13 N or \n BLC0080501_SAP_POSTBG 1 2 3 4 5 6 7 8 9 10 11 12 13 E <Group ID> ");
			printGenevaMessage("ERROR", SAP_POSTBG_PROG_NAME, "MAIN", str_msglog);
			exit(EXIT_FAIL);
	}

	GetDateTime(str_sysdate,(char *)"%Y/%m/%d-%H-%M-%S");

	// retrieve module parameter
	if( bl_sap_postbg_getparameter() == BL_SAP_POSTBG_NOTOK )
	{
		exit(EXIT_FAIL);
	}

	//Check Log directory is existing.
	DIR * dir_p;
	if ( (dir_p = opendir(str_log_path)) == NULL )
	{
		sprintf(str_msglog,"Not found Log path: %s", str_log_path);
		printGenevaMessage("ERROR", SAP_POSTBG_PROG_NAME, "main", str_msglog);
		exit(EXIT_FAIL);
	}
	closedir(dir_p);

	// open log file
	file_logfp = bl_open_LogFile_pid(str_log_path, str_log_name, getpid());
	if( file_logfp == NULL )
	{
		sprintf(str_msglog,"Cannot create log file");
		printGenevaMessage("ERROR", SAP_POSTBG_PROG_NAME, "main", str_msglog);
		exit(EXIT_FAIL);
	}
	strcpy(env_database,str_GNVdbconn);

	
	sprintf(str_msglog,"START TIME: %s",str_sysdate);
	printGenevaMessage("INFORM", SAP_POSTBG_PROG_NAME, "MAIN", str_msglog);
	bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG, "MAIN", str_msglog);
	
	sprintf(str_msglog,"BLC0080501_SAP_POSTBG version : %s",str_version);
	printGenevaMessage("INFORM", SAP_POSTBG_PROG_NAME, "MAIN", str_msglog);
	bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG, "MAIN", str_msglog);

	
	otl_connect::otl_initialize(); // initialize OCI environment
try{
		// connect to GENEVA Oracle 
		db.rlogon(env_database);
		printGenevaMessage("INFORM", SAP_POSTBG_PROG_NAME, "MAIN", "CONNECT TO DATABASE SUCCESSFUL.");

		int_V_NEW_GROUP_ID = -1;
		otl_stream stream_update_inv_newgroup_ccc(1, SQL_UPDATE_41_2, db); //kittipop 21/10/2008
		stream_update_inv_newgroup_ccc	<< int_V_NEW_GROUP_ID << int_V_GROUP_ID;

		sprintf(str_msglog,"Update New SAP Ext Group ID(CCC) in table INV_SUMMARY from [%d] to [%d]", int_V_GROUP_ID, int_V_NEW_GROUP_ID);
		printGenevaMessage("INFORM", SAP_POSTBG_PROG_NAME, "MAIN", str_msglog);
		bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"SQL 41_2",str_msglog);
		db.commit();

		otl_stream stream_select_nxt_grid(1024, SQL_SELECT_0, db);
		if( stream_select_nxt_grid.eof() )
		{
			//error
			sprintf(str_msglog,"Seq Gen for CC_SEQ_PC_SAP_EXT_GROUP_ID not found");
			printGenevaMessage("ERROR", SAP_POSTBG_PROG_NAME, "MAIN", str_msglog);
			bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG, "MAIN", str_msglog);

			exit(EXIT_FAIL);
		}
		
		stream_select_nxt_grid >> int_V_NEW_GROUP_ID;

		if(	bl_sap_postbg_SQL_1_4() == BL_SAP_POSTBG_NOTOK )
		{
			// ERROR in Step 1 Generate Extraction Sequence
			exit(EXIT_FAIL);
		}

		otl_stream stream_select_all_rev(1024, SQL_SELECT_5, db);
		while (!stream_select_all_rev.eof())
		{
			int_count++;
			int_Status = BL_SAP_POSTBG_OK;

			stream_select_all_rev >> str_V_ExtractSeq >> int_V_ICOID >> str_V_ExtractDat >> long_V_TotalRevenue >> int_TOTInv >> str_V_VatDate >> str_V_BillType >> str_V_Status;
		
			sprintf(str_msglog,"=== PROCEED REVENUE EXTRACTION FOR [%s] ===",str_V_ExtractSeq);
			printGenevaMessage("INFORM", SAP_POSTBG_PROG_NAME, "MAIN", str_msglog);
			bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG, "MAIN", str_msglog);
			
			if(long_V_TotalRevenue == NULL )
			{
				long_V_TotalRevenue = 0;
			}
			// CUR_REVCODE
			if(	bl_sap_postbg_SQL_6_8() == BL_SAP_POSTBG_OK )
			{
			// CLOSE CUR_REVCODE
				otl_stream stream_select_all_aud_rev(1024, SQL_SELECT_9, db);
				stream_select_all_aud_rev << str_V_ExtractSeq << int_V_ICOID;
					
				if(!stream_select_all_aud_rev.eof())
				{
					// Print error to LOG file
					sprintf(str_msglog,"ERROR: REVENUE CODE DOES NOT HAVE REGION BREAKDOWN EXTRACTION_SEQ [%s]",str_V_ExtractSeq);
					printGenevaMessage("ERROR", SAP_POSTBG_PROG_NAME, "MAIN", str_msglog);
					bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG, "MAIN", str_msglog);
					bl_sap_postbg_RAISE_ERROR();
				}

				// Temp variable for internal use
				int  int_tmp_SQL10_status = 0;
				char str_tmp_SQL10_err_ID[MAX_BUFFER] = "";
				char str_tmp_data[MAX_BUFFER] = "";

				otl_stream stream_select_val_reg_cri(1024, SQL_SELECT_10, db);
				stream_select_val_reg_cri << str_V_ExtractSeq << int_V_ICOID;

				while(!stream_select_val_reg_cri.eof())
				{
					stream_select_val_reg_cri >> str_tmp_data;

					if( int_tmp_SQL10_status == 1 )
						strcat(str_tmp_SQL10_err_ID,", ");

					strcat(str_tmp_SQL10_err_ID,str_tmp_data);
					int_tmp_SQL10_status = 1;
				}
						
				if( int_tmp_SQL10_status == 1 )
				{
					// Print error to LOG file
					sprintf(str_msglog,"ERROR: SOME OF THE REVENUE[%s] CODES FOR DISCOUNT SPECIFIED RETURN MORE THAN ONE REVENUE CODE TO OFFSSET WITH",str_tmp_SQL10_err_ID);
					printGenevaMessage("ERROR", SAP_POSTBG_PROG_NAME, "MAIN", str_msglog);				
					bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG, "MAIN", str_msglog);
					bl_sap_postbg_RAISE_ERROR();
					continue;
				}
			}	//bl_sap_postbg_SQL_6_8()
			
			if (int_Status == BL_SAP_POSTBG_HANDLE)
			{
				// Mark Complete and continue next extraction sequence
				int_Status = BL_SAP_POSTBG_OK;
				bl_sap_postbg_UPDATE_SUCCESS();
				continue;
			}			
			if (int_Status == BL_SAP_POSTBG_NOTOK)
			{
				// Raise Error and continue next extraction sequence
				bl_sap_postbg_RAISE_ERROR();
				continue;
			}				
			
			if(	bl_sap_postbg_SQL_11_12() == BL_SAP_POSTBG_NOTOK )
			{
				// Error found Raise error and continue next extraction sequence
				bl_sap_postbg_RAISE_ERROR();
				continue;
			}
			//================================================
			otl_stream stream_select_bd_all_rev(1024, SQL_SELECT_13, db);
			stream_select_bd_all_rev << str_V_ExtractSeq;

			if( stream_select_bd_all_rev.eof() )
			{
				sprintf(str_msglog,"DATA NOT FOUND FOR EXTRACTION SEQUENCE [%s]", str_V_ExtractSeq);
				printGenevaMessage("WARN", SAP_POSTBG_PROG_NAME, "MAIN", str_msglog);
				bl_Log_Print(file_logfp,BLSAPPOSTBG_WARNING_LOG, "MAIN", str_msglog);
				bl_sap_postbg_RAISE_ERROR();
				continue;
			}
			while(!stream_select_bd_all_rev.eof())
			{
				// Clear CUR_SAP value
				int_CUR_SAP_SAP_Ext_Group_ID = 0;
				int_CUR_SAP_Revenue_Code_ID = 0;
				strcpy(str_CUR_SAP_Revenue_Code_Name,"");
				long_CUR_SAP_Orig_Revenue_MNY = 0;
				int_CUR_SAP_Region_Criteria = 0;
				char_CUR_SAP_Derived_Ratio_Used =' ';
				strcpy(str_CUR_SAP_Revenue_Type,"");
				strcpy(str_CUR_SAP_Extraction_SEQ,"");

				stream_select_bd_all_rev >> int_CUR_SAP_SAP_Ext_Group_ID >> int_CUR_SAP_Revenue_Code_ID >> str_CUR_SAP_Revenue_Code_Name >> long_CUR_SAP_Orig_Revenue_MNY >> int_CUR_SAP_Region_Criteria >> char_CUR_SAP_Derived_Ratio_Used >>str_CUR_SAP_Revenue_Type >> str_CUR_SAP_Extraction_SEQ;

				sprintf(str_msglog,"BREAK DOWN GROUP ID [%d] REVENUE CODE ID[%d] REVENUE CODE NAME[%s]", int_CUR_SAP_SAP_Ext_Group_ID, int_CUR_SAP_Revenue_Code_ID, str_CUR_SAP_Revenue_Code_Name);
		//		printGenevaMessage("WARN", SAP_POSTBG_PROG_NAME, "MAIN", str_msglog);
				bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG, "MAIN", str_msglog);					
//----------------------------------------------------------------------------------------------------------			
				if( (int_CUR_SAP_Region_Criteria == 1) && (strcmp(str_CUR_SAP_Revenue_Type,"RE") == 0 ) )
				{
					int_Status = bl_sap_postbg_case_1RE();
					if( int_Status == BL_SAP_POSTBG_NOTOK )
					{
						bl_sap_postbg_RAISE_ERROR();
					}
				} // End case REGION_CRITERIA = 1 && REVENUE_TYPE = "RE"
//----------------------------------------------------------------------------------------------------------
				else if( (int_CUR_SAP_Region_Criteria == 2) && (strcmp(str_CUR_SAP_Revenue_Type,"RE") == 0 ) )
				{
					int_Status = bl_sap_postbg_case_2RE();
					if( int_Status == BL_SAP_POSTBG_NOTOK )
					{
						bl_sap_postbg_RAISE_ERROR();
					}
				} // End case REGION_CRITERIA = 2 && REVENUE_TYPE = "RE"
//----------------------------------------------------------------------------------------------------------
				else if( (int_CUR_SAP_Region_Criteria == 3) && (strcmp(str_CUR_SAP_Revenue_Type,"RE") == 0 ) )
				{
					int_Status = bl_sap_postbg_case_3RE();
					if( int_Status == BL_SAP_POSTBG_NOTOK )
					{
						bl_sap_postbg_RAISE_ERROR();
					}
				} // End case REGION_CRITERIA = 3 && REVENUE_TYPE = "RE"
//----------------------------------------------------------------------------------------------------------				
				else if( (int_CUR_SAP_Region_Criteria == 4) && (strcmp(str_CUR_SAP_Revenue_Type,"RE") == 0 ) )
				{
					int_Status = bl_sap_postbg_case_4RE();
					if( int_Status == BL_SAP_POSTBG_NOTOK )
					{
						bl_sap_postbg_RAISE_ERROR();
					}
				} // End case REGION_CRITERIA = 4 && REVENUE_TYPE = "RE"
				else if( (int_CUR_SAP_Region_Criteria > 5) && (strcmp(str_CUR_SAP_Revenue_Type,"RE") == 0 ) )
				{
					int_Status = bl_sap_postbg_case_MORE5RE();
					if( int_Status == BL_SAP_POSTBG_NOTOK )
					{
						bl_sap_postbg_RAISE_ERROR();
					}
				} // End case REGION_CRITERIA > 5 && REVENUE_TYPE = "RE"
//----------------------------------------------------------------------------------------------------------				
				else if( (int_CUR_SAP_Region_Criteria == 3) && (strcmp(str_CUR_SAP_Revenue_Type,"DS") == 0 ) )
				{
					int_Status = bl_sap_postbg_case_3DS();
					if( int_Status == BL_SAP_POSTBG_NOTOK )
					{
						bl_sap_postbg_RAISE_ERROR();
					}
				} // End case REGION_CRITERIA = 3 && REVENUE_TYPE = "DS"
//----------------------------------------------------------------------------------------------------------
				else if( (int_CUR_SAP_Region_Criteria == 5) && ((strcmp(str_CUR_SAP_Revenue_Type,"DS") == 0) || (strcmp(str_CUR_SAP_Revenue_Type,"DG") == 0) ) )
				{
					int_Status = bl_sap_postbg_case_5DSDG();
					if( int_Status == BL_SAP_POSTBG_NOTOK )
					{
						bl_sap_postbg_RAISE_ERROR();
					}
				} // End case REGION_CRITERIA = 5 && REVENUE_TYPE = "RE"
//----------------------------------------------------------------------------------------------------------				
				else // Region Criteria and Revenue Type Unknown
				{
					sprintf(str_msglog,"Unknown Region Criteria [%d] and REVENUE CODE [%s]", int_CUR_SAP_Region_Criteria, str_CUR_SAP_Revenue_Type );
					printGenevaMessage("INFORM",SAP_POSTBG_PROG_NAME, "MAIN", str_msglog);
					bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG, "MAIN", str_msglog);
					bl_sap_postbg_RAISE_ERROR();
					int_Status = BL_SAP_POSTBG_NOTOK;
				}
			} // end while CUR_SAP

			if(	int_Status == BL_SAP_POSTBG_OK )
			{
				int_Status = bl_sap_postbg_check_diff_mny();
			}		
					
			if( int_Status == BL_SAP_POSTBG_NOTOK )
			{
				bl_sap_postbg_RAISE_ERROR();
			}
			else
			{
				bl_sap_postbg_UPDATE_SUCCESS();			
			}
						
			db.commit();
			sprintf(str_msglog,"=============================================");
			printGenevaMessage("INFORM", SAP_POSTBG_PROG_NAME, "MAIN", str_msglog);
			bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG, "MAIN", str_msglog);
		}//while
		
		// Error Handling Mode
		if( int_Mode == ERROR_MODE )
		{
				otl_stream stream_update_invalid(1, SQL_UPDATE_40, db);
				stream_update_invalid<<int_V_GROUP_ID;
				
				sprintf(str_msglog,"Update SAP Ext Group ID [%d] status to 'INVALID'", int_V_GROUP_ID);
				bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"SQL 40",str_msglog);
		}
		
	sprintf(str_msglog,"TOTAL LINES POCESS WITHCONDITION: EXTRACTION_DAT IS NULL IN CC_TBL_DAT_SAP_AUDIT_REV_EXT is [%d]", int_count);
	bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"MAIN",str_msglog);
	printGenevaMessage("INFORM",SAP_POSTBG_PROG_NAME, "MAIN",str_msglog);
	
}catch(otl_exception& p){ // intercept OTL exceptions

		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.msg);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.stm_text);
		printGenevaMessage("ERROR",SAP_POSTBG_PROG_NAME, "otl_exception",(char*)p.var_info);

		sprintf(str_msglog,"SQL ERROR [%s]",(char*)p.stm_text);
		bl_Log_Print(file_logfp,BLSAPPOSTBG_ERROR_LOG,"MAIN",str_msglog);

		int_Status = BL_SAP_POSTBG_NOTOK;
		//db.rollback();

		GetDateTime(str_sysdate,(char *)"%Y/%m/%d-%H-%M-%S");
		sprintf(str_msglog,"FINISH TIME: %s",str_sysdate);

		printGenevaMessage("INFORM",SAP_POSTBG_PROG_NAME, "MAIN",str_msglog);
		bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE,"MAIN",str_msglog);
	
		//fclose(file_logfp);
		//db.logoff();
	} // end catch

	if ( int_Status == BL_SAP_POSTBG_NOTOK )
	{
		db.rollback();
	}
	else
	{
		db.commit();
	}

	otl_stream stream_update_inv_newgroup(1, SQL_UPDATE_41_1, db);
	stream_update_inv_newgroup<< int_V_NEW_GROUP_ID<< int_V_GROUP_ID;

	sprintf(str_msglog,"Update New SAP Ext Group ID in table INV_SUMMARY from [%d] to [%d]", int_V_GROUP_ID, int_V_NEW_GROUP_ID);
	bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"SQL 41_1",str_msglog);
	printGenevaMessage("INFORM",SAP_POSTBG_PROG_NAME, "MAIN",str_msglog);
	
	GetDateTime(str_sysdate,(char *)"%Y/%m/%d-%H-%M-%S");
	sprintf(str_msglog,"FINISH TIME: %s",str_sysdate);

	printGenevaMessage("INFORM",SAP_POSTBG_PROG_NAME, "MAIN",str_msglog);
	bl_Log_Print(file_logfp,BLSAPPOSTBG_MESSAGE_LOG,"MAIN",str_msglog);
	
	db.commit();
	fclose(file_logfp);
	db.logoff();

	// check return error
	if( int_fwd_status == 1 )
	{
		exit(EXIT_FAIL);
	}

	exit(EXIT_SUCCESS);
}//main
