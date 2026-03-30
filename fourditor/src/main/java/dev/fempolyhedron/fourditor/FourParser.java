// fourditor
//
// Authors:
//  - fempolyhedron :3
//    * github  : https://github.com/FemboyPolyhedron
//    * website : https://fempolyhedron.dev
//    * bsky    : poly.fempolyhedron.dev
//                (poly.bskypds.fempolyhedron.dev)
//    * discord : @elephant_lover
//    * twitter : @FemPolyhedron
//
// (c) fempolyhedron 2026
//
// four :3
package dev.fempolyhedron.fourditor;

import org.w3c.dom.*;
import javax.xml.parsers.*;
import java.io.File;
import java.util.*;

public class FourParser
{
    public static List<FourElement> parse(File file) throws Exception
    {
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        DocumentBuilder builder = factory.newDocumentBuilder();
        Document doc = builder.parse(file);
        doc.getDocumentElement().normalize();

        Element root = doc.getDocumentElement();
        if (!"four".equals(root.getTagName()))
            throw new IllegalArgumentException(
                "root element must be <four>, got <" + root.getTagName() + ">"
            );

        return childElements(root);
    }

    private static List<FourElement> childElements(Element parent)
    {
        List<FourElement> result = new ArrayList<>();
        NodeList nodes = parent.getChildNodes();
        for (int i = 0; i < nodes.getLength(); i++)
        {
            Node node = nodes.item(i);
            if (node.getNodeType() == Node.ELEMENT_NODE) result.add(parseElement((Element) node));
        }
        return result;
    }

    private static FourElement parseElement(Element el)
    {
        String tag = el.getTagName();

        Map<String, String> attrs = new LinkedHashMap<>();
        NamedNodeMap attrMap = el.getAttributes();
        for (int i = 0; i < attrMap.getLength(); i++)
        {
            Node a = attrMap.item(i);
            attrs.put(a.getNodeName(), a.getNodeValue());
        }

        String id    = attrs.remove("id");
        String clazz = attrs.remove("class");
        if (clazz == null) clazz = attrs.remove("clazz");

        String bodyText = null;
        List<FourElement> children = new ArrayList<>();
        NodeList nodes = el.getChildNodes();
        for (int i = 0; i < nodes.getLength(); i++)
        {
            Node node = nodes.item(i);
            if (node.getNodeType() == Node.TEXT_NODE)
            {
                String t = node.getTextContent().strip();
                if (!t.isEmpty()) bodyText = t;
            }
            else if (node.getNodeType() == Node.ELEMENT_NODE)
            {
                children.add(parseElement((Element) node));
            }
        }

        String contentAttr = attrs.remove("content");
        String content = contentAttr != null ? contentAttr : bodyText;

        return new FourElement(tag, id, clazz, content, attrs, children);
    }
}
