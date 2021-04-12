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
 * - Sergey Vasilevskiy, Doug Johnson
 */

/******************************************************************************
*   Description:        Implementation of the Security input file that scales
*                       with the database size.
******************************************************************************/

#ifndef SECURITY_FILE_H
#define SECURITY_FILE_H

#include <map>
#include "CompanyFile.h"

using namespace std;

namespace TPCE
{

// We use a small set of values for 26 raised to a power, so store them in
// a constant array to save doing calls to pow( 26.0, ? )
static const UINT  Power26[] = { 1, 26, 676, 17576, 456976, 11881376, 308915776 };

// For index i > 0, this array holds the sum of 26^0 ... 26^(i-1)
static const UINT64  Power26Sum[] = { 0, 1, 27, 703, 18279, 475255, 12356631, 321272407, UINT64_CONST(8353082583) };

class CSecurityFile : public CFlatFile<TSecurityInputRow, TSecurityLimits>
{
    // Total number of securities in the database.
    // Depends on the total number of customers.
    //
    TIdent  m_iConfiguredSecurityCount;
    TIdent  m_iActiveSecurityCount;

    // Number of base companies (=rows in Company.txt input file).
    //
    UINT    m_iBaseCompanyCount;

    // Used to map a symbol to it's id value
    bool                    m_SymbolToIdMapIsLoaded;
    map< string, TIdent >   m_SymbolToIdMap;
    map< char, int >        m_LowerCaseLetterToIntMap;
    char                    m_SUFFIX_SEPARATOR;

    void CreateSuffix( TIdent Multiplier, char* pBuf, size_t BufSize )
    {
        size_t CharCount( 0 );
        INT64 Offset( 0 );
        INT64 LCLIndex( 0 );    // LowerCaseLetter array index

        while( (UINT64)Multiplier >= Power26Sum[CharCount+1] )
        {
            CharCount++;
        }

        if( CharCount + 2 <= BufSize )  // 1 extra for separator and 1 extra for terminating NULL
        {
            *pBuf = m_SUFFIX_SEPARATOR;
            pBuf++;
            // CharCount is the number of letters needed in the suffix
            // The base string is a string of 'a's of length CharCount
            // Find the offset from the base value represented by the string
            // of 'a's to the desired number, and modify the base string
            // accordingly.
            Offset = Multiplier - Power26Sum[CharCount];

            while( CharCount > 0 )
            {
                LCLIndex = Offset / Power26[ CharCount-1 ];
                *pBuf = LowerCaseLetters[ LCLIndex ];
                pBuf++;
                Offset -= ( LCLIndex * Power26[ CharCount-1 ] );
                CharCount--;
            }
            *pBuf = '\0';
        }
        else
        {
            // Not enough room in the buffer
            CharCount = BufSize - 1;
            while( CharCount > 0 )
            {
                *pBuf = m_SUFFIX_SEPARATOR;
                pBuf++;
                CharCount--;
            }
            *pBuf = '\0';
        }
    }

    INT64 ParseSuffix( const char* pSymbol )
    {
        int CharCount( 0 );
        INT64 Multiplier( 0 );

        CharCount = (int) strlen( pSymbol );

        Multiplier = Power26Sum[CharCount];

        while( CharCount > 0 )
        {
            Multiplier += (INT64)Power26[ CharCount-1 ] * 
                           m_LowerCaseLetterToIntMap[ *pSymbol ];
            CharCount--;
            pSymbol++;
        }
        return( Multiplier );
    }

public:

    // Constructor.
    //
    CSecurityFile(const char *szListFile, TIdent iConfiguredCustomerCount, TIdent iActiveCustomerCount)
    : CFlatFile<TSecurityInputRow, TSecurityLimits>(szListFile)
    , m_SymbolToIdMapIsLoaded( false )
    {
        TCompanyLimits      CompanyInputFileLimits;

        m_SUFFIX_SEPARATOR = '-';
        SetConfiguredSecurityCount(iConfiguredCustomerCount);
        SetActiveSecurityCount(iActiveCustomerCount);

        m_iBaseCompanyCount     = CompanyInputFileLimits.m_iTotalElements;
    }

    // Constructor.
    //
    CSecurityFile(const string &str, TIdent iConfiguredCustomerCount, TIdent iActiveCustomerCount)
    : CFlatFile<TSecurityInputRow, TSecurityLimits>(str)
    , m_SymbolToIdMapIsLoaded( false )
    {
        TCompanyLimits      CompanyInputFileLimits;

        m_SUFFIX_SEPARATOR = '-';
        SetConfiguredSecurityCount(iConfiguredCustomerCount);
        SetActiveSecurityCount(iActiveCustomerCount);

        m_iBaseCompanyCount     = CompanyInputFileLimits.m_iTotalElements;
    }

    // Delayed initialization of the configured number of securities.
    // Made available for situations when the configured number of customers
    // is not known at construction time.
    //
    void SetConfiguredSecurityCount(TIdent iConfiguredCustomerCount)
    {
        m_iConfiguredSecurityCount = CalculateSecurityCount(iConfiguredCustomerCount);
    }

    // Delayed initialization of the active number of securities.
    // Made available for situations when the active number of customers
    // is not known at construction time.
    //
    void SetActiveSecurityCount(TIdent iActiveCustomerCount)
    {
        m_iActiveSecurityCount = CalculateSecurityCount(iActiveCustomerCount);
    }

    // Calculate total security count for the specified number of customers.
    // Sort of a static method. Used in parallel generation of securities related tables.
    //
    TIdent CalculateSecurityCount(TIdent iCustomerCount)
    {
        return iCustomerCount / iDefaultLoadUnitSize * iOneLoadUnitSecurityCount;
    }

    // Calculate the first security id (0-based) for the specified customer id
    //
    TIdent CalculateStartFromSecurity(TIdent iStartFromCustomer)
    {
        return iStartFromCustomer / iDefaultLoadUnitSize * iOneLoadUnitSecurityCount;
    }

    // Create security symbol with mod/div magic.
    //
    // This function is needed to scale unique security
    // symbols with the database size.
    //
    void CreateSymbol(  TIdent  iIndex,     // row number
                        char*   szOutput,   // output buffer
                        size_t  iOutputLen) // size of the output buffer (including null)
    {
        TIdent  iFileIndex  = iIndex % CFlatFile<TSecurityInputRow, TSecurityLimits>::GetSize();
        TIdent  iAdd        = iIndex / CFlatFile<TSecurityInputRow, TSecurityLimits>::GetSize();
        size_t  iNewLen;

        // Load the base symbol
        strncpy(szOutput, GetRecord(iFileIndex)->S_SYMB, iOutputLen);

        szOutput[iOutputLen - 1] = '\0';    // Ensure NULL termination

        // Add a suffix if needed
        if (iAdd > 0)
        {
            iNewLen = strlen( szOutput );
            CreateSuffix( iAdd, &szOutput[iNewLen], iOutputLen - iNewLen );
        }
    }

    // Return company id for the specified row of the SECURITY table.
    // Index can exceed the size of the Security flat file.
    //
    TIdent GetCompanyId(TIdent iIndex)
    {
        // Index wraps around every 6850 securities (5000 companies).
        //
        return m_list[ (int)(iIndex % m_list.size()) ].S_CO_ID  + iTIdentShift
                + iIndex / m_list.size() * m_iBaseCompanyCount;
    }

    TIdent GetCompanyIndex( TIdent Index )
    {
        // Indices and Id's are offset by 1
        return( GetCompanyId( Index ) - 1 - iTIdentShift);
    }

    // Return the number of securities in the database for
    // a certain number of customers.
    //
    TIdent GetSize()
    {
        return m_iConfiguredSecurityCount;
    }

    // Return the number of securities in the database for
    // the configured number of customers.
    //
    TIdent GetConfiguredSecurityCount()
    {
        return m_iConfiguredSecurityCount;
    }

    // Return the number of securities in the database for
    // the active number of customers.
    //
    TIdent GetActiveSecurityCount()
    {
        return m_iActiveSecurityCount;
    }

    // Overload GetRecord to wrap around indices that
    // are larger than the flat file
    //
    TSecurityInputRow*  GetRecord(TIdent index) { return &m_list[(int)(index % m_list.size())]; };

    // Load the symbol-to-id map
    bool LoadSymbolToIdMap( void )
    {
        if( ! m_SymbolToIdMapIsLoaded )
        {
            int ii;

            for( ii = 0; ii < m_list.size(); ii++ )
            {
                string sSymbol( m_list[ii].S_SYMB );
                m_SymbolToIdMap[ sSymbol ] = m_list[ii].S_ID;
            }
            m_SymbolToIdMapIsLoaded = true;

            for( ii = 0; ii < MaxLowerCaseLetters; ii++ )
            {
                m_LowerCaseLetterToIntMap[ LowerCaseLetters[ii] ] = ii;
            }
        }
        return( m_SymbolToIdMapIsLoaded );
    }

    TIdent GetId( char* pSymbol )
    {
        char*   pSeparator( NULL );

        if( !m_SymbolToIdMapIsLoaded )
        {
            LoadSymbolToIdMap();
        }
        if( NULL == ( pSeparator = strchr( pSymbol, m_SUFFIX_SEPARATOR )))
        {
            // we're dealing with a base symbol
            string sSymbol(pSymbol);
            return( m_SymbolToIdMap[ sSymbol ] );
        }
        else
        {
            // we're dealing with an extended symbol
            char*   pSuffix( NULL );
            TIdent  BaseId( 0 );
            TIdent  Multiplier( 0 );

            string  sSymbol(pSymbol, static_cast<size_t>(pSeparator-pSymbol));
            BaseId = m_SymbolToIdMap[ sSymbol ];

            pSuffix = pSeparator + 1;               // The suffix starts right after the separator character
            Multiplier = (int)ParseSuffix( pSuffix );// For now, suffix values fit in an int, cast ParseSuffix to avoid compiler warning

            return(( Multiplier * m_list.size() ) + BaseId );
        }
    }

    TIdent GetIndex( char* pSymbol )
    {
        // Indices and Id's are offset by 1
        return( GetId( pSymbol ) - 1 );
    }

    eExchangeID GetExchangeIndex( TIdent index )
    {
        // The mod converts a scaled security index into a base security index
        char *pExchange = m_list[(int)(index % m_list.size())].S_EX_ID;
        eExchangeID eExchangeIndex = eNYSE;  // XXX(tzwang): just give it NYSE to silence a GCC warning

        if (!strcmp(pExchange, "NYSE"))
        {
            eExchangeIndex = eNYSE;
        }
        else if (!strcmp(pExchange, "NASDAQ"))
        {
            eExchangeIndex = eNASDAQ;
        }
        else if (!strcmp(pExchange, "AMEX"))
        {
            eExchangeIndex = eAMEX;
        }
        else if (!strcmp(pExchange, "PCX"))
        {
            eExchangeIndex = ePCX;
        }
        else
        {
            assert(false);
        }

        return eExchangeIndex;
    }
};

}   // namespace TPCE

#endif // SECURITY_FILE_H

 