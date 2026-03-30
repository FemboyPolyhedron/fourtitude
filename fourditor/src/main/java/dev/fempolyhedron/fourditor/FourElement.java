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

import java.util.List;
import java.util.Map;

public class FourElement
{
    private final String              tag;
    private final String              id;
    private final String              clazz;
    private final String              content;
    private final Map<String, String> attrs;
    private final List<FourElement>   children;

    public FourElement(String tag, String id, String clazz, String content, Map<String, String> attrs, List<FourElement> children)
    {
        this.tag      = tag;
        this.id       = id;
        this.clazz    = clazz;
        this.content  = content;
        this.attrs    = attrs;
        this.children = children;
    }

    public String              tag()      { return tag; }
    public String              id()       { return id; }
    public String              clazz()    { return clazz; }
    public String              content()  { return content; }
    public Map<String, String> attrs()    { return attrs; }
    public List<FourElement>   children() { return children; }
}
