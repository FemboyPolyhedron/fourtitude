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

import java.nio.file.Path;
import java.util.*;

public class CppCodegen
{
    private static final Map<String, String> ELEMENT_TYPES = ElementRegistry.buildMap();

    private final List<String> fields = new ArrayList<>();
    private final List<String> inits  = new ArrayList<>();
    private int counter = 0;

    public String generate(String xmlFileName, List<FourElement> roots)
    {
        String structName = structNameFrom(xmlFileName);

        for (FourElement el : roots) processElement(el, null);

        StringBuilder sb = new StringBuilder();
        sb.append("// generated from fourditor //");
        sb.append("#pragma once\n#include \"fourtitude/fourtitudecpp.h\"\n");
        sb.append("struct ").append(structName).append("{");
        for (String f : fields) sb.append(f);
        sb.append("Fourtitude::FourtitudeDocument document;");
        sb.append(structName).append("(){");
        for (String line : inits) sb.append(line);
        sb.append("}};");
        return sb.toString();
    }

    private void processElement(FourElement el, String parentVar)
    {
        String cppType = ELEMENT_TYPES.get(el.tag());
        if (cppType == null)
        {
            System.err.println("fourditor: warning: unknown element <" + el.tag() + ">, skipping");
            return;
        }

        String var = el.id() != null ? toIdentifier(el.id()) : "_el" + counter++;
        fields.add(cppType + " " + var + ";");

        if (el.id()    != null) inits.add(var + ".raw()->id = "    + cppStr(el.id())    + ";");
        if (el.clazz() != null) inits.add(var + ".raw()->clazz = " + cppStr(el.clazz()) + ";");

        Map<String, String> a = el.attrs();
        String x = a.get("x"), y = a.get("y"), w = a.get("width"), h = a.get("height");
        if (x != null && y != null && w != null && h != null) inits.add(var + ".setBounds(" + x + ", " + y + ", " + w + ", " + h + ");");
        else if (x != null && y != null && w != null) { inits.add(var + ".setPosition(" + x + ", " + y + ");"); inits.add(var + ".raw()->width = " + w + ";"); }
        else if (x != null && y != null && h != null) { inits.add(var + ".setPosition(" + x + ", " + y + ");"); inits.add(var + ".raw()->height = " + h + ";"); }
        else if (x != null && y != null) inits.add(var + ".setPosition(" + x + ", " + y + ");");
        else if (w != null && h != null) inits.add(var + ".setSize(" + w + ", " + h + ");");
        else if (w != null) inits.add(var + ".raw()->width = " + w + ";");
        else if (h != null) inits.add(var + ".raw()->height = " + h + ";");

        String enabled = a.get("enabled");
        if (enabled != null)
        {
            boolean val = enabled.equals("1") || enabled.equalsIgnoreCase("true");
            inits.add(var + ".setEnabled(" + val + ");");
        }

        if (el.content() != null) inits.add(var + ".raw()->content = " + cppStr(el.content()) + ";");

        String src = a.get("src");
        if (src != null) inits.add(var + ".setSrc(" + cppStr(src) + ");");

        Set<String> KNOWN = Set.of("x", "y", "width", "height", "enabled", "src");
        for (Map.Entry<String, String> entry : a.entrySet())
        {
            if (KNOWN.contains(entry.getKey())) continue;
            inits.add(var + ".raw()->" + toIdentifier(entry.getKey()) + " = " + cppValue(entry.getValue()) + ";");
        }

        inits.add("document.append(&" + var + ");");
        if (parentVar != null) inits.add(parentVar + ".appendChild(&" + var + ");");

        for (FourElement child : el.children()) processElement(child, var);
    }

    static String structNameFrom(String xmlFileName)
    {
        String name = Path.of(xmlFileName).getFileName().toString();
        for (String suffix : new String[]{".four.xml", ".xml"})
        {
            if (name.endsWith(suffix))
            {
                name = name.substring(0, name.length() - suffix.length());
                break;
            }
        }
        return name.replaceAll("[^a-zA-Z0-9]", "_") + "_fourxml";
    }

    static String toIdentifier(String s)
    {
        return s.replaceAll("[^a-zA-Z0-9_]", "_");
    }

    static String cppStr(String s)
    {
        return "\"" + s.replace("\\", "\\\\").replace("\"", "\\\"").replace("\n", "\\n") + "\"";
    }

    /** Emits integers, floats, and booleans as raw C++ literals; everything else as a quoted string. */
    static String cppValue(String s)
    {
        if (s.equals("true") || s.equals("false")) return s;
        try { Integer.parseInt(s); return s; } catch (NumberFormatException ignored) {}
        try { Float.parseFloat(s); return s + (s.contains(".") ? "f" : ".0f"); } catch (NumberFormatException ignored) {}
        return cppStr(s);
    }
}
