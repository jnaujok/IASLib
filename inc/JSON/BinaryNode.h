#ifndef IASLIB_BINARYNODE_H__
#define IASLIB_BINARYNODE_H__

#ifdef IASLIB_JSON_SUPPORT__

#include "ValueNode.h"
#include "DataBlock.h"

namespace IASLib {
    class CBinaryNode : public CValueNode {
    private:
        CDataBlock  *dataBlock;
    public:
        DEFINE_OBJECT(CBinaryNode)

        CBinaryNode( CString name, CString value );

        virtual ~CBinaryNode();

        virtual void setValue( CString value );
    };
}

#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_BINARYNODE_H__
