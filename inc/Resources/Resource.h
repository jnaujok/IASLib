/***********************************************************************
**  Resource Class
**
**  Description:
**      This class gives us access to a resource contained inside of 
** the compiled resource file. It also is used when the resource file
** is generated. A resource is normally returned to the program from
** a call to the Resource File class.
**
**      Note that typed resources are derived from this class.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

#ifndef IASLIB_RESOURCE_H__
#define IASLIB_RESOURCE_H__

#include "../BaseTypes/Object.h"
#include "../BaseTypes/IASLib_String.h"
#include "../BaseTypes/Date.h"
#include "ResourceFile.h"

namespace IASLib
{
    class CResource : public CObject
    {
        protected:
            union
            {
                struct
                {
                    short int       m_nFolderTag;
                    short int       m_nResourceTag;
                };
                long int            m_nResourceID;
            } m_ID;
            CString                 m_strName;
            CDate                   m_dttLastModified;
            CString                 m_strModifiedBy;
            long                    m_nDataLength;
            int                     m_nType;
            long                    m_nReserved;
            bool                    m_bCompressed;
            long                    m_nCompressedLength;
            void                   *m_pData;

        public:
                                    CResource( void );
                                    CResource( short int nFolder, short int nResource );
                                    CResource( short int nFolder, short int nResource, const char *strName, const char *strModifier, int nType, const char *strFilename );

            virtual                ~CResource( void );

            virtual bool            Load( short int nFolder, short int nResource );
            virtual bool            Save( void );
    };
} // namespace IASLib

#endif // IASLIB_RESOURCE_H__
