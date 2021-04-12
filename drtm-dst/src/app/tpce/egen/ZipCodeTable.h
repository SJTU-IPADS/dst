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
 * - Doug Johnson
 */

/*
*   Class representing the ZIP_CODE table.
*/
#ifndef ZIP_CODE_TABLE_H
#define ZIP_CODE_TABLE_H

#include "EGenTables_common.h"

namespace TPCE
{

class CZipCodeTable : public TableTemplate<ZIP_CODE_ROW>
{
private:
    TZipCodeFile*       m_ZipCode;
    TZipCodeInputRow*   m_NextRow;
    UINT32              m_NextUniqueRecordID;

public:
    CZipCodeTable( CInputFiles inputFiles )
        : TableTemplate<ZIP_CODE_ROW>()
        , m_ZipCode( inputFiles.ZipCode )
        , m_NextUniqueRecordID( 0 )
    {
        // No body for constructor.
    };


    /*
    *   Generates all column values for the next row.
    */
    bool GenerateNextRecord()
    {
        if( m_NextUniqueRecordID < m_ZipCode->RecordCount( ))
        {
            strncpy( m_row.ZC_CODE, (m_ZipCode->GetRecordByPassKey( m_NextUniqueRecordID ))->ZC_CODE, sizeof( m_row.ZC_CODE ) );
            strncpy( m_row.ZC_TOWN, (m_ZipCode->GetRecordByPassKey( m_NextUniqueRecordID ))->ZC_TOWN, sizeof( m_row.ZC_TOWN ) );
            strncpy( m_row.ZC_DIV, (m_ZipCode->GetRecordByPassKey( m_NextUniqueRecordID ))->ZC_DIV, sizeof( m_row.ZC_DIV ) );
            m_NextUniqueRecordID++;
            return( true );
        }
        else
        {
            return ( false );
        }
    }
};

}   // namespace TPCE

#endif //ZIP_CODE_TABLE_H
 