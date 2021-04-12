/*
 * Legal Notice
 *
 * This document and associated source code (the "Work") is a part of a
 * benchmark specification maintained by the TPC.
 *
 * The TPC reserves all right, title, and interest to the Work as provided
 * under U.S. and international laws, including without limitation all patent
 * and trademark rights therein.
 *
 * No Warranty
 *
 * 1.1 TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THE INFORMATION
 *     CONTAINED HEREIN IS PROVIDED "AS IS" AND WITH ALL FAULTS, AND THE
 *     AUTHORS AND DEVELOPERS OF THE WORK HEREBY DISCLAIM ALL OTHER
 *     WARRANTIES AND CONDITIONS, EITHER EXPRESS, IMPLIED OR STATUTORY,
 *     INCLUDING, BUT NOT LIMITED TO, ANY (IF ANY) IMPLIED WARRANTIES,
 *     DUTIES OR CONDITIONS OF MERCHANTABILITY, OF FITNESS FOR A PARTICULAR
 *     PURPOSE, OF ACCURACY OR COMPLETENESS OF RESPONSES, OF RESULTS, OF
 *     WORKMANLIKE EFFORT, OF LACK OF VIRUSES, AND OF LACK OF NEGLIGENCE.
 *     ALSO, THERE IS NO WARRANTY OR CONDITION OF TITLE, QUIET ENJOYMENT,
 *     QUIET POSSESSION, CORRESPONDENCE TO DESCRIPTION OR NON-INFRINGEMENT
 *     WITH REGARD TO THE WORK.
 * 1.2 IN NO EVENT WILL ANY AUTHOR OR DEVELOPER OF THE WORK BE LIABLE TO
 *     ANY OTHER PARTY FOR ANY DAMAGES, INCLUDING BUT NOT LIMITED TO THE
 *     COST OF PROCURING SUBSTITUTE GOODS OR SERVICES, LOST PROFITS, LOSS
 *     OF USE, LOSS OF DATA, OR ANY INCIDENTAL, CONSEQUENTIAL, DIRECT,
 *     INDIRECT, OR SPECIAL DAMAGES WHETHER UNDER CONTRACT, TORT, WARRANTY,
 *     OR OTHERWISE, ARISING IN ANY WAY OUT OF THIS OR ANY OTHER AGREEMENT
 *     RELATING TO THE WORK, WHETHER OR NOT SUCH AUTHOR OR DEVELOPER HAD
 *     ADVANCE NOTICE OF THE POSSIBILITY OF SUCH DAMAGES.
 *
 * Contributors
 * - Sergey Vasilevskiy
 */

#ifndef EGEN_TABLES_STDAFX_H
#define EGEN_TABLES_STDAFX_H

#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers

#include <cassert>
#include <cmath>
#include <fstream>
#include <set>
#include <map>
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <queue>

// TODO: reference additional headers your program requires here
using namespace std;

#include "EGenStandardTypes.h"
#include "EGenUtilities_stdafx.h"
#include "InputFile.h"
#include "InputFileNoWeight.h"
#include "FlatFile.h"
#include "Table_Defs.h"
#include "InputFlatFilesDeclarations.h"
#include "SecurityFile.h"
#include "CompanyFile.h"
#include "CompanyCompetitorFile.h"
#include "InputFlatFilesStructure.h"
#include "TableTemplate.h"
#include "Person.h"
#include "CustomerSelection.h"
#include "CustomerTable.h"
#include "CompanyTable.h"   //must be before Address and Financial tables
#include "FinancialTable.h"
#include "AddressTable.h"
#include "CustomerAccountsAndPermissionsTable.h"
#include "CustomerTaxratesTable.h"
#include "HoldingsAndTradesTable.h"
#include "WatchListsAndItemsTable.h"
#include "SecurityTable.h"
#include "DailyMarketTable.h"
#include "Brokers.h"
#include "SectorTable.h"
#include "ChargeTable.h"
#include "ExchangeTable.h"
#include "CommissionRateTable.h"
#include "IndustryTable.h"
#include "StatusTypeTable.h"
#include "TaxrateTable.h"
#include "TradeTypeTable.h"
#include "CompanyCompetitorTable.h"
#include "ZipCodeTable.h"
#include "NewsItemAndXRefTable.h"
#include "MEESecurity.h"    // must be before LastTradeTable.h
#include "LastTradeTable.h"
#include "TradeGen.h"

#endif  // #ifndef EGEN_TABLES_STDAFX_H
 