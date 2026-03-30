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

import java.io.File;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;

public class Main
{
    public static void main(String[] args)
    {
        if (args.length != 2)
        {
            System.err.println("usage: fourditor <input.four.xml> <output.hpp>");
            System.exit(1);
        }

        String xmlPath = args[0];
        String outPath = args[1];

        try
        {
            List<FourElement> elements = FourParser.parse(new File(xmlPath));

            String xmlFileName = Path.of(xmlPath).getFileName().toString();
            String output = new CppCodegen().generate(xmlFileName, elements);

            Path out = Path.of(outPath);
            if (out.getParent() != null) Files.createDirectories(out.getParent());
            Files.writeString(out, output);

            System.out.println("fourditor: " + xmlPath + " -> " + outPath + "  [" + CppCodegen.structNameFrom(xmlFileName) + "]");
        }
        catch (Exception e)
        {
            System.err.println("fourditor: error: " + e.getMessage());
            System.exit(1);
        }
    }
}
