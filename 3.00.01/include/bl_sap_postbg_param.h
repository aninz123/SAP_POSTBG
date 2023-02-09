/* IRB Upgrade v3.0: No Impact on Upgrade to IRB 3.0 : Jagdeep S Virdi  */
/*Added Version Identification*/

#ifndef __SAP_POSTBG_PARAM_H__
#define __SAP_POSTBG_PARAM_H__ 

/*  UPGRADE IRB v3.0: Added for Version Identification */
char ac_bl_sap_postbg_param_Version[] = "@(#) bl_sap_postbg_param.h Version 3.0";

#define EXIT_SUCCESS				0
#define EXIT_FAIL					1
#define EXIT_WARNING				2

#define BL_SAP_POSTBG_OK			0
#define BL_SAP_POSTBG_NOTOK			1
#define BL_SAP_POSTBG_WARNING       -2
#define BL_SAP_POSTBG_HANDLE		2

#define MAX_BUFFER					1024

using namespace std;

// need change in the future
#define SAP_POSTBG_PROG_NAME		"BLC0080501SAP_POSTBG"

#define BLSAPPOSTBG_ENVCONFIGPATH	"$GNV_BATCH"
#define BLSAPPOSTBG_CONFIGFILE		"/batch/config/SAP_POSTBG/BLC0080501MainConfig.cfg"

// Message Type
#define BLSAPPOSTBG_ERROR_LOG		"E"			// Error log type
#define BLSAPPOSTBG_MESSAGE_LOG		"M"			// Message log type
#define BLSAPPOSTBG_WARNING_LOG		"W"			// Warning log type
#define BLSAPPOSTBG_ERROR			"Error"		// Error log type
#define BLSAPPOSTBG_MESSAGE			"Message"	// Message log type
#define BLSAPPOSTBG_WARNING			"Warning"	// Warning log type
#define BLERROR						0
#define BLMESSAGE					1
#define BLWARNING					2

#define GENEVA_DBCONN				"DATABASESTRING"
#define GLOBAL_CONFIG_FILE			"GlobalFullFileName"
#define LOG_PATH					"LogPath"
#define LOG_FILE					"PrefixLogFile"

// Running Mode
#define NORMAL_MODE					1
#define ERROR_MODE					0

// Error Message
#define ERR_MSG_1		"Environment variable not found"
#define ERR_MSG_2		"Cannot open configuration file"
#define ERR_MSG_3		"Cannot open global config value from the configuration file"
#define ERR_MSG_4		"Cannot get the Log path from the configuration file"
#define ERR_MSG_5		"Cannot get the Log file name from the configuration file"
#define ERR_MSG_10		"ICO_Name for ICO_ID [%d] not found : SQL [%s]"
#define ERR_MSG_11		"Data from SQL 6 not found : SQL [%s]"
#define ERR_MSG_12		"Can not find Region_Criteria in CC_TBL_DAT_SAP_REV_CRITERIA ICOID[%d] REVENUE_CODE_ID [%d] : SQL [%s]"
#define ERR_MSG_13		"Region Code from SQL 11_1 not found"
#define ERR_MSG_14		"TOTAL REGION MONEY not found : SQL [%s]"
#define ERR_MSG_15		"NO DATA FROM CC_TBL_DAT_SAP_AUDIT_RATIO : SQL [%s]"
#define ERR_MSG_16		"MATH EXCEPTION: DIVIDE BY ZERO"
#define ERR_MSG_17		"SUM OF REVENUE MONEY NOT FOUND : SQL [%s]"
#define ERR_MSG_18		"SAP REGION ID NOT FOUND FROM CC_TBL_DAT_SAP_REGION_MAP : SQL [%s]"
#define ERR_MSG_19		"SUM OF REVENUE MONEY NOT FOUND : SQL [%s]"
#define ERR_MSG_20		"SAP REGION ID NOT FOUND : SQL [%s]"
#define ERR_MSG_21		"CHARGE_RC_ID NOT FOUND : SQL [%s]"
#define ERR_MSG_22		"SUM OF DISCOUNT MONEY NOT FOUND : SQL [%s]"
#define ERR_MSG_23		"RECORD NOT FOUND FROM CC_TBL_DAT_SAP_AUDIT_BYREGION : SQL [%s]"
#define ERR_MSG_24		"REVCODE VALUE NOT FOUND FROM CC_TBL_DAT_VARIABLES WHERE NAME =[%s]"
#define ERR_MSG_25		"REGION VALUE NOT FOUND FROM CC_TBL_DAT_VARIABLES WHERE NAME =[%s]"

#define ERR_MSG_26		"Not found discount adjustment in CC_TBL_DAT_VARIABLES: SQL [%s]"
#define ERR_MSG_27		"Split Discount Adjustment Revenue Error : Value [%s]"

// ====   SQL STATEMENT   ================
#define SQL_UPDATE_SAP_AUDIT_REV_EXT		"UPDATE CC_TBL_DAT_SAP_AUDIT_REV_EXT " \
								"SET EXTRACTION_DAT = TO_DATE(:f1<char[40]>,'DD/MM/YYYY'), " \
								"TOT_REVENUE_MNY = :f2<long>, STATUS = :f3<char[40]> " \
								"WHERE EXTRACTION_SEQ = :f4<char[40]>"

#define SQL_UPDATE_SAP_AUDIT_REV_EXT_SUCCESS		"UPDATE CC_TBL_DAT_SAP_AUDIT_REV_EXT " \
								"SET EXTRACTION_DAT = TO_DATE(:f1<char[40]>,'DD/MM/YYYY'), " \
								"TOT_REVENUE_MNY = :f2<long>, STATUS = :f3<char[40]> " \
								"WHERE EXTRACTION_SEQ = :f4<char[20]> " \
								"AND STATUS <> 'FAILED' "
								
#define SQL_SELECT_1			"SELECT COUNT(A.VAT_DATE) \"TOT_INV\", TO_CHAR(A.VAT_DATE,'DD/MM/YYYY'), " \
								"		DECODE(B.BILL_TYPE_ID, 1, 'PERIODIC', 3, 'PERIODIC', 4, 'TERMINATION', 7, 'TERMINATION') BILLTYPENAME, " \
								"		C.INVOICING_CO_ID " \
								"FROM CC_TBL_DAT_INV_SUMMARY A, BILLSUMMARY B, INVOICINGCOMPANY C " \
								"WHERE A.SAP_EXT_GROUP_ID = :f1<int> " \
								"AND A.ACCOUNT_NUM = B.ACCOUNT_NUM " \
								"AND A.BILL_SEQUENCE = B.BILL_SEQ " \
								"AND B.BILL_TYPE_ID IN (1,3,4,7) " \
								"AND A.INVOICING_CO_NAME = C.INVOICING_CO_NAME " \
								"AND A.INVOICE_TYPE = 'IN' " \
								"GROUP BY A.VAT_DATE, DECODE(B.BILL_TYPE_ID, 1, 'PERIODIC', 3, 'PERIODIC',  4, 'TERMINATION', 7, 'TERMINATION'), C.INVOICING_CO_ID " \
								"ORDER BY C.INVOICING_CO_ID ASC, DECODE(B.BILL_TYPE_ID, 1, 'PERIODIC', 3, 'PERIODIC',  4, 'TERMINATION', 7, 'TERMINATION') ASC, A.VAT_DATE ASC"
								
#define SQL_SELECT_2			"SELECT MAX(TO_NUMBER(SUBSTR(EXTRACTION_SEQ,1,2))) \"V_YEAR\" " \
								"FROM CC_TBL_DAT_SAP_AUDIT_REV_EXT"

#define SQL_SELECT_2_1			"SELECT TO_CHAR(SYSDATE,'YY') FROM DUAL"

#define SQL_SELECT_3			"SELECT MAX(TO_NUMBER(SUBSTR(EXTRACTION_SEQ,-7))) +1 \"V_EXTRACTSEQ\" " \
								"FROM CC_TBL_DAT_SAP_AUDIT_REV_EXT"
								
#define SQL_INSERT_4			"INSERT INTO CC_TBL_DAT_SAP_AUDIT_REV_EXT " \
								"(SAP_EXT_GROUP_ID, EXTRACTION_SEQ, INVOICING_CO_ID, EXTRACTION_DAT, TOT_REVENUE_MNY, INVOICE_COUNT, VAT_DATE, BILL_TYPE_NAME, STATUS) " \
								"VALUES(:f1<int>, TO_CHAR(SYSDATE,'YY-') || :f2<char[20]>,:f3<int>,NULL,NULL,:f4<long>,TO_DATE(:f5<char[40]>,'DD/MM/YYYY'),:f6<char[40]>,'PENDING')"
								
#define SQL_SELECT_5_1			"SELECT INVOICING_CO_NAME FROM INVOICINGCOMPANY " \
													"WHERE INVOICING_CO_ID =  :f1<int>"
// RBM 9.0.4 Upgrade
// Satvashila Nikam : changed code for billdetails.REVENUE_CODE_ID
// RBM 9.0.4, billdetails.REVENUE_CODE_ID(int) has been replaced with billdetails.GROUP_ATTR_1(varchar(40))
// Also tye of the column is changed.
// Also the billdetails.revenue_mny is mapped to SUM_ATTR_1_VALUE 
// INVOICING_CO_NAME filed length has been changed to 120 from 40
// 	remove condition AND A.GROUP_ATTR_1 = CAST(D.REVENUE_CODE_ID as VARCHAR2(40)) 20180109 prissana.p													
#define SQL_SELECT_6			"SELECT SUM(A.SUM_ATTR_1_VALUE) \"TOT_REVENUE\", D.REVENUE_CODE_ID, "\
									"D.REVENUE_CODE_NAME "\
									"FROM BILLDETAILS A, BILLSUMMARY B, CC_TBL_DAT_INV_SUMMARY C, REVENUECODE D "\
									"WHERE A.ACCOUNT_NUM = B.ACCOUNT_NUM "\
									"AND A.ACCOUNT_NUM = C.ACCOUNT_NUM "\
									"AND A.BILL_SEQ = B.BILL_SEQ "\
									"AND B.ACCOUNT_NUM = C.ACCOUNT_NUM "\
									"AND B.BILL_SEQ = C.BILL_SEQUENCE "\
									"AND A.REVENUE_FEED_ID = -1 "\								
									"AND to_number(A.GROUP_ATTR_1) = D.REVENUE_CODE_ID  "\ 
									"AND (B.BILL_TYPE_ID = :f1<int> OR B.BILL_TYPE_ID = :f2<int>) "\
									"AND C.SAP_EXT_GROUP_ID = :f3<int> "\
									"AND C.VAT_DATE = TO_DATE(:f4<char[40]>,'DD/MM/YYYY') "\
									"AND C.INVOICING_CO_NAME = :f5<char[120]> "\
									"AND C.INVOICE_TYPE = 'IN' "\
								"GROUP BY D.REVENUE_CODE_NAME , D.REVENUE_CODE_ID " 

#define SQL_SELECT_7			"SELECT DERIVE_RATIO_USE \"V_RATIO\", REGION_CRITERIA \"V_REGION\", " \
									"REVENUE_TYPE \"V_REVTYPE\" " \
								"FROM CC_TBL_DAT_SAP_REV_CRITERIA " \
								"WHERE INVOICING_CO_ID =  :f1<int> " \
									"AND REVENUE_CODE_ID = :f2<int>"
									
#define SQL_INSERT_8			"INSERT INTO CC_TBL_DAT_SAP_AUDIT_REVCODE " \
								"(SAP_EXT_GROUP_ID, REVENUE_CODE_ID, REVENUE_CODE_NAME, ORIG_REVENUE_MNY, REGION_CRITERIA, DERIVED_RATIO_USED, REVENUE_TYPE, EXTRACTION_SEQ) " \
								"VALUES(:f1<int>, :f2<int>,:f3<char[40]>, :f4<long>, :f5<int>,:f6<char[20]>,:f7<char[20]>,:f8<char[20]>)"	

#define SQL_SELECT_11_1			"SELECT DISTINCT(SUBSTR(REVENUE_CODE_NAME,-2)) \"REGIONCODE\" " \
								"FROM CC_TBL_DAT_SAP_AUDIT_REVCODE " \
								"WHERE DERIVED_RATIO_USED = 'Y' AND EXTRACTION_SEQ = :f1<char[20]> "
								
#define SQL_SELECT_11_2			"SELECT SUM(ORIG_REVENUE_MNY) \"TOT_REVENUE\" " \
								"FROM CC_TBL_DAT_SAP_AUDIT_REVCODE " \
								"WHERE DERIVED_RATIO_USED = 'Y' AND EXTRACTION_SEQ = :f1<char[20]> " \
								"AND SUBSTR(REVENUE_CODE_NAME,-2) = :f2<char[40]>"								

#define SQL_SELECT_12			"SELECT * FROM CC_TBL_DAT_SAP_AUDIT_RATIO " \
								"WHERE EXTRACTION_SEQ = :f1<char[20]> " \
								"AND REGION_CODE = :f2<char[20]>"

#define SQL_UPDATE_12_1			"UPDATE CC_TBL_DAT_SAP_AUDIT_RATIO " \
								"SET SAP_EXT_GROUP_ID =  :f1<int>, REGION_MNY =  :f2<long> " \
								"WHERE EXTRACTION_SEQ =  :f3<char[20]> AND REGION_CODE = :f4<char[20]>"

#define SQL_INSERT_12_2			"INSERT INTO CC_TBL_DAT_SAP_AUDIT_RATIO " \
								"(SAP_EXT_GROUP_ID, EXTRACTION_SEQ, REGION_CODE, REGION_MNY) " \
								"VALUES(:f1<int>, :f2<char[20]>, :f3<char[20]>, :f4<long>)"

#define SQL_SELECT_ORIGMNY_SAP_AUDIT_BYREGION		"SELECT ORIG_REVENUE_MNY \"V_ORIGMNY\" " \
								"FROM CC_TBL_DAT_SAP_AUDIT_BYREGION " \
								"WHERE EXTRACTION_SEQ = :f1<char[20]>" \
									"AND REGION_CODE = :f2<char[20]> " \
									"AND REVENUE_CODE_ID =  :f3<int>"
									
#define SQL_UPDATE_SAP_AUDIT_BYREGION			"UPDATE CC_TBL_DAT_SAP_AUDIT_BYREGION " \
								"SET SAP_EXT_GROUP_ID = :f1<int>, ORIG_REVENUE_MNY = :f2<long>, "\
								" POST_SPECIFIED_DISCOUNTED_MNY = :f3<long>, " \
								" POST_GENERAL_DISCOUNTED_MNY = :f4<long>" \
								" WHERE EXTRACTION_SEQ = :f5<char[20]> " \
								" AND REGION_CODE = :f6<char[20]>" \
								" AND REVENUE_CODE_ID = :f7<int>"
																							
#define	SQL_INSERT_SAP_AUDIT_BYREGION		"INSERT INTO CC_TBL_DAT_SAP_AUDIT_BYREGION " \
								"(SAP_EXT_GROUP_ID, REVENUE_CODE_ID, REGION_CODE, ORIG_REVENUE_MNY, POST_SPECIFIED_DISCOUNTED_MNY, POST_GENERAL_DISCOUNTED_MNY, EXTRACTION_SEQ) " \
								"VALUES(:f1<int>, :f2<int>, :f3<char[20]>, :f4<long>, :f5<long>, :f6<long>, :f7<char[20]>)"

#define SQL_SELECT_16			"SELECT SUM(REGION_MNY) \"V_REGIONTOTAL\" " \
								"FROM CC_TBL_DAT_SAP_AUDIT_RATIO " \
								"WHERE EXTRACTION_SEQ =:f1<char[20]>"
								
#define SQL_SELECT_17			"SELECT * FROM CC_TBL_DAT_SAP_AUDIT_RATIO " \
								"WHERE EXTRACTION_SEQ = :f1<char[20]>"								
								
#define SQL_SELECT_17_1          "SELECT INTEGER_VALUE FROM CC_TBL_DAT_VARIABLES "\
			                        "WHERE module = 'BLC0080000' "\
		                                "AND name = :f1<char[255]> "\
			                        "AND effective_date <= sysdate "\
			                        "AND ((expired_date >= sysdate) or expired_date is null) "

// RBM 9.0.4 Upgrade
// Satvashila Nikam : changed code for billdetails.REVENUE_CODE_ID
// RBM 9.0.4, billdetails.REVENUE_CODE_ID(int) has been replaced with billdetails.GROUP_ATTR_1(varchar(40))
//Also the billdetails.revenue_mny is mapped to SUM_ATTR_1_VALUE
					
#define SQL_SELECT_20			"SELECT /*+ full(a) parallel(a,8) */ SUM(A.SUM_ATTR_1_VALUE), E.REGION " \
								" FROM billdetails a, accountattributes e, cc_tbl_dat_inv_summary c " \
								" WHERE c.VAT_DATE = TO_DATE(:f1<char[40]>,'DD/MM/YYYY') " \
								" AND c.SAP_EXT_GROUP_ID = :f2<int> " \
								" AND c.INVOICING_CO_NAME = :f3<char[120]> " \
								" AND a.ACCOUNT_NUM = c.ACCOUNT_NUM " \
								" AND a.BILL_SEQ = c.bill_sequence " \
								" AND a.GROUP_ATTR_1 = :f4<char[40]> " \
								" AND a.REVENUE_FEED_ID = -1 "\
								" AND e.ACCOUNT_NUM = a.ACCOUNT_NUM " \
								" AND exists " \
								" ( select * from billsummary b " \
								" where a.account_num = b.account_num " \
								" AND a.bill_seq = b.bill_seq " \
								" AND (b.BILL_TYPE_ID = :f5<int> OR b.BILL_TYPE_ID = :f6<int>) " \
								"  ) " \
								"  AND exists " \
								"  ( select * from revenuecode d "\
								"  where d.REVENUE_CODE_ID = :f7<int> " \
								"  ) "\
								" GROUP BY e.REGION"

#define SQL_SELECT_UNIQUE_SAP_REGION_MAP			"SELECT UNIQUE SAP_REGION_ID \"V_REGION\" " \
								"FROM CC_TBL_DAT_SAP_REGION_MAP " \
								"WHERE REGION_CODE = :f1<char[20]> " \
								"AND INVOICING_CO_ID = :f2<int>"
																																					
#define SQL_SELECT_42			"SELECT REGION_ID " \
								"FROM CC_TBL_DAT_SAP_CRITERIA_REGION " \
								"WHERE INVOICING_CO_ID =  :f1<int> " \
								"AND REGION_CRITERIA =  :f2<int>"\
								"AND REVENUE_TYPE =  :f3<char[20]> "

#define SQL_SELECT_43			"SELECT ORIG_REVENUE_MNY  FROM CC_TBL_DAT_SAP_AUDIT_BYREGION " \
								"WHERE EXTRACTION_SEQ = :f1<char[20]> " \
									"AND REGION_CODE = :f2<char[20]> " \
									"AND REVENUE_CODE_ID =  :f3<int> "								
		
#define SQL_SELECT_27N			"SELECT CHARGE_REVENUE_CODE_ID \"V_CHARGERCID\" " \
								"FROM CC_TBL_DAT_SAP_DISCOUNT_RULES " \
								"WHERE DISCOUNT_REVENUE_CODE_ID = :f1<int> " \
								"AND INVOICING_CO_ID = :f2<int>"		
				
#define SQL_SELECT_28			"SELECT POST_SPECIFIED_DISCOUNTED_MNY \"V_PSDISCOUNT\" " \
								"FROM CC_TBL_DAT_SAP_AUDIT_BYREGION " \
								"WHERE EXTRACTION_SEQ =:f1<char[20]> " \
									"AND REGION_CODE = :f2<char[20]> " \
									"AND REVENUE_CODE_ID = :f3<int>"
																																			                 
#define SQL_UPDATE_29_1			"UPDATE CC_TBL_DAT_SAP_AUDIT_BYREGION " \
								"SET SAP_EXT_GROUP_ID =:f1<int>, POST_SPECIFIED_DISCOUNTED_MNY = :f2<long>,POST_GENERAL_DISCOUNTED_MNY = :f3<long> " \
								"WHERE EXTRACTION_SEQ = :f4<char[20]> " \
									"AND REGION_CODE = :f5<char[20]> " \
									"AND REVENUE_CODE_ID = :f6<int>"																																                 
																																			                 
#define	SQL_INSERT_SAP_AUDIT_REV_DISC			"INSERT INTO CC_TBL_DAT_SAP_AUDIT_REV_DISC " \
								"(SAP_EXT_GROUP_ID, EXTRACTION_SEQ, REVENUE_CODE_ID, DISCOUNT_FROM, DISCOUNT_MNY, REVENUE_TYPE, REGION_CODE) " \
								"VALUES(:f1<int>, :f2<char[20]>,  :f3<int>,  :f4<int>,  :f5<long>, :f6<char[20]>, :f7<char[20]>)"							

#define SQL_SELECT_30			"SELECT SUM(POST_SPECIFIED_DISCOUNTED_MNY) \"V_TOTALPSMNY\", " \
									"SUM(POST_GENERAL_DISCOUNTED_MNY) \"V_TOTALPGMNY\" " \
								"FROM CC_TBL_DAT_SAP_AUDIT_BYREGION " \
								"WHERE EXTRACTION_SEQ = :f1<char[20]> AND REVENUE_CODE_ID IN " \
									"(SELECT CHARGE_REVENUE_CODE_ID " \
									"FROM CC_TBL_DAT_SAP_DISCOUNT_RULES " \
									"WHERE DISCOUNT_REVENUE_CODE_ID = :f2<int> " \
									"AND INVOICING_CO_ID = :f3<int>) " \
								"AND POST_SPECIFIED_DISCOUNTED_MNY > 0 " \
								"AND POST_GENERAL_DISCOUNTED_MNY > 0 "								

#define SQL_SELECT_30_1			"SELECT STRING_VALUE "\
								"FROM CC_TBL_DAT_VARIABLES "\
								"WHERE MODULE = 'BLC0080000' "\
								"AND NAME = 'SAP_DISC_ADJ_REVENUE' "

#define SQL_SELECT_30_2			"SELECT * FROM CC_TBL_DAT_SAP_AUDIT_BYREGION "\
								"WHERE EXTRACTION_SEQ =  :f1<char[20]> "\
								"AND REVENUE_CODE_ID = :f2<int> "
								
	#define SQL_INSERT_30_3			"INSERT INTO CC_TBL_DAT_SAP_AUDIT_BYREGION "\
								"(SAP_EXT_GROUP_ID, REVENUE_CODE_ID, REGION_CODE, ORIG_REVENUE_MNY, POST_SPECIFIED_DISCOUNTED_MNY, POST_GENERAL_DISCOUNTED_MNY, EXTRACTION_SEQ) " \
								"VALUES(:f1<int>, :f2<int>, '01', 0, 0, 0, :f3<char[20]>)"							
	
#define SQL_SELECT_31			"SELECT * FROM CC_TBL_DAT_SAP_AUDIT_BYREGION " \
								"WHERE EXTRACTION_SEQ = :f1<char[20]> AND REVENUE_CODE_ID IN " \
									"(SELECT CHARGE_REVENUE_CODE_ID " \
									"FROM CC_TBL_DAT_SAP_DISCOUNT_RULES " \
									"WHERE DISCOUNT_REVENUE_CODE_ID = :f2<int> " \
									"AND INVOICING_CO_ID = :f3<int>)"	
	
#define SQL_SELECT_32			"SELECT ORIG_REVENUE_MNY FROM CC_TBL_DAT_SAP_AUDIT_BYREGION " \
								"WHERE EXTRACTION_SEQ =  :f1<char[20]> " \
									"AND REGION_CODE =  :f2<char[20]> " \
									"AND REVENUE_CODE_ID = :f3<int>"					
									
#define SQL_UPDATE_33_1			"UPDATE CC_TBL_DAT_SAP_AUDIT_BYREGION " \
								"SET SAP_EXT_GROUP_ID =  :f1<int>, POST_SPECIFIED_DISCOUNTED_MNY = :f2<long>, POST_GENERAL_DISCOUNTED_MNY = :f3<long> " \
								"WHERE EXTRACTION_SEQ =  :f4<char[20]>" \
									"AND REGION_CODE =  :f5<char[20]> " \
									"AND REVENUE_CODE_ID = :f6<int>"									
									
#define SQL_UPDATE_33_2			"UPDATE CC_TBL_DAT_SAP_AUDIT_BYREGION " \
								"SET SAP_EXT_GROUP_ID =  :f1<int>, POST_GENERAL_DISCOUNTED_MNY = :f2<long> " \
								"WHERE EXTRACTION_SEQ =  :f3<char[20]> " \
									"AND REGION_CODE = :f4<char[20]> " \
									"AND REVENUE_CODE_ID = :f5<int>"
									
#define SQL_SELECT_34			"SELECT SUM(POST_GENERAL_DISCOUNTED_MNY) \"V_TOTALDERIVEDREVENUE\" " \
								"FROM CC_TBL_DAT_SAP_AUDIT_BYREGION " \
								"WHERE EXTRACTION_SEQ =:f1<char[20]>"
								
#define SQL_SELECT_35			"SELECT INTEGER_VALUE  \"V_ REVCODEVALUE\" " \
								"FROM CC_TBL_DAT_VARIABLES " \
								"WHERE MODULE = 'BLC0080000' " \
									"AND NAME =:f1<char[255]> " \
									"AND EFFECTIVE_DATE <= SYSDATE " \
									"AND ((EXPIRED_DATE >= SYSDATE) OR EXPIRED_DATE IS NULL)"
									
#define SQL_SELECT_36			"SELECT INTEGER_VALUE \"V_REGIONVALUE\" " \
								"FROM CC_TBL_DAT_VARIABLES " \
								"WHERE MODULE = 'BLC0080000' " \
									"AND NAME = :f1<char[255]> " \
									"AND EFFECTIVE_DATE <= SYSDATE " \
									"AND ((EXPIRED_DATE >= SYSDATE) OR EXPIRED_DATE IS NULL)"
									
#define SQL_SELECT_37_0			"SELECT ORIG_REVENUE_MNY, POST_SPECIFIED_DISCOUNTED_MNY, POST_GENERAL_DISCOUNTED_MNY " \
								"FROM CC_TBL_DAT_SAP_AUDIT_BYREGION " \
								"WHERE SAP_EXT_GROUP_ID = :f1<int> AND EXTRACTION_SEQ =:f2<char[20]> " \
								"AND REVENUE_CODE_ID = :f3<long> " \
								"AND REGION_CODE = :f4<char[20]>"									

#define SQL_UPDATE_37			"UPDATE CC_TBL_DAT_SAP_AUDIT_BYREGION " \
								"SET ORIG_REVENUE_MNY = :f1<long>, POST_SPECIFIED_DISCOUNTED_MNY = :f2<long>, POST_GENERAL_DISCOUNTED_MNY = :f3<long> " \
								"WHERE SAP_EXT_GROUP_ID = :f4<int> AND EXTRACTION_SEQ = :f5<char[20]> " \
								"AND REVENUE_CODE_ID = :f6<long> " \
								"AND REGION_CODE = :f7<char[20]>"

#define SQL_UPDATE_41_1			"UPDATE CC_TBL_DAT_INV_SUMMARY " \
								"SET SAP_EXT_GROUP_ID = :f1<int> " \
								"WHERE SAP_EXT_GROUP_ID = :f2<int> " \
								" AND INVOICE_TYPE = 'IN' "
	
#define SQL_UPDATE_41_2			"UPDATE CC_TBL_DAT_INV_SUMMARY " \
														"SET SAP_EXT_GROUP_ID = :f1<int> " \
														"WHERE SAP_EXT_GROUP_ID = :f2<int> " \
														" AND INVOICE_TYPE <> 'IN' "				
														
#define SQL_SELECT_0			"SELECT CC_SEQ_PC_SAP_EXT_GROUP_ID.NEXTVAL FROM DUAL"		

#define SQL_SELECT_5			"SELECT EXTRACTION_SEQ, INVOICING_CO_ID, TO_CHAR(SYSDATE, 'DD/MM/YYYY'), TOT_REVENUE_MNY, INVOICE_COUNT, TO_CHAR(VAT_DATE,'DD/MM/YYYY'), BILL_TYPE_NAME, STATUS " \
								"FROM CC_TBL_DAT_SAP_AUDIT_REV_EXT " \
								"WHERE EXTRACTION_DAT IS NULL"

#define SQL_SELECT_9			"SELECT * FROM CC_TBL_DAT_SAP_AUDIT_REVCODE " \
									"WHERE EXTRACTION_SEQ = :f1<char[20]> " \
									"AND (DERIVED_RATIO_USED = 'Y' OR (REGION_CRITERIA = 1 AND REVENUE_TYPE = 'RE')) " \
									"AND (SUBSTR(REVENUE_CODE_NAME,-2) NOT IN (SELECT REGION_ID FROM CC_TBL_DAT_SAP_REGION " \
																			   "WHERE INVOICING_CO_ID = :f2<int>) " \
										"OR LENGTH(REVENUE_CODE_NAME) <> 12 " \
										"OR SUBSTR(REVENUE_CODE_NAME,1,2) <> 'ET')"

#define SQL_SELECT_10			"SELECT DISCOUNT_REVENUE_CODE_ID " \
								"FROM CC_TBL_DAT_SAP_DISCOUNT_RULES " \
								"WHERE DISCOUNT_REVENUE_CODE_ID  IN " \
									"(SELECT REVENUE_CODE_ID " \
									"FROM CC_TBL_DAT_SAP_AUDIT_REVCODE " \
									"WHERE REGION_CRITERIA = 3 AND REVENUE_TYPE = 'DS' " \
										"AND EXTRACTION_SEQ = :f1<char[20]> ) " \
								"AND INVOICING_CO_ID = :f2<int>  " \
								"GROUP BY DISCOUNT_REVENUE_CODE_ID " \
								"HAVING COUNT(CHARGE_REVENUE_CODE_ID) > 1"
								
#define	SQL_SELECT_13			"SELECT * FROM CC_TBL_DAT_SAP_AUDIT_REVCODE " \
								"WHERE EXTRACTION_SEQ = :f1<char[20]> " \
								"ORDER BY REVENUE_TYPE DESC, REGION_CRITERIA ASC"
								
#define SQL_UPDATE_40			"UPDATE CC_TBL_DAT_SAP_AUDIT_REV_EXT " \
								"SET STATUS = 'INVALID' " \
								"WHERE SAP_EXT_GROUP_ID = :f1<int>"
																																																																																																																																										                        																		
#endif
