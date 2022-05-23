#ifndef IASLIB_BOOLEANNODE_H__
#define IASLIB_BOOLEANNODE_H__

#ifdef IASLIB_JSON_SUPPORT__

#include "ValueNode.h"

namespace IASLib {
    class CBooleanNode : public CValueNode {
    private:
        bool bValue;
        bool bIsNull;
    public:
        DEFINE_OBJECT(CBooleanNode)

        CBooleanNode( CJsonNode *parent, CString name, CString value );

        void setValue( bool value );
        void setNull()
        {
            bIsNull = true;
        }

        /**
         * Method that will try to convert value of this node to a boolean value.
         */
        virtual bool asBoolean();
        virtual bool asBoolean(bool defaultValue);

        virtual CString	asText();

        virtual bool equals(CJsonNode *o);
    };
}

#endif // IASLIB_JSON_SUPPORT__

#endif // IASLIB_BOOLEANNODE_H__
