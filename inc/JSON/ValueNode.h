#ifndef IASLIB_VALUENODE_H__
#define IASLIB_VALUENODE_H__

#ifdef IASLIB_JSON_SUPPORT__

#include "JsonNode.h"
#include "ObjectNode.h"

namespace IASLib {
    class CValueNode : public CJsonNode {
    public:
        DEFINE_OBJECT(CValueNode)

    protected:
        CJsonNode *findParent(String fieldName);

        TArray<CJsonNode>	findParents(String fieldName, List<JsonNode> foundSoFar)
        CJsonNode *findValue(String fieldName)
        Method for finding a JSON Object field with specified name in this node or its child nodes, and returning value it has.
        List<JsonNode>	findValues(String fieldName, List<JsonNode> foundSoFar)
        List<String>	findValuesAsText(String fieldName, List<String> foundSoFar)
        JsonNode	get(int index)
        Method for accessing value of the specified element of an array node.
        JsonNode	get(String fieldName)
        Method for accessing value of the specified field of an object node.
        boolean	has(int index)
        Method that allows checking whether this node is JSON Array node and contains a value for specified index If this is the case (including case of specified indexing having null as value), returns true; otherwise returns false.
        boolean	has(String fieldName)
        Method that allows checking whether this node is JSON Object node and contains value for specified property.
        boolean	hasNonNull(int index)
        Method that is similar to JsonNode.has(int), but that will return false for explicitly added nulls.
        boolean	hasNonNull(String fieldName)
        Method that is similar to JsonNode.has(String), but that will return false for explicitly added nulls.
        JsonNode	path(int index)
        This method is similar to JsonNode.get(int), except that instead of returning null if no such element exists (due to index being out of range, or this node not being an array), a "missing node" (node that returns true for JsonNode.isMissingNode()) will be returned.
        JsonNode	path(String fieldName)
        This method is similar to JsonNode.get(String), except that instead of returning null if no such value exists (due to this node not being an object, or object not having value for the specified field), a "missing node" (node that returns true for JsonNode.isMissingNode()) will be returned.
        void	serializeWithType(JsonGenerator jg, SerializerProvider provider, TypeSerializer typeSer)
        Type information is needed, even if JsonNode instances are "plain" JSON, since they may be mixed with other types.
        String	toString()
        Note: marked as abstract to ensure all implementation classes define it properly.
    };
}

#endif // IASLIB_JSON_SUPPORT__

#endif // IASLIB_VALUENODE_H__
