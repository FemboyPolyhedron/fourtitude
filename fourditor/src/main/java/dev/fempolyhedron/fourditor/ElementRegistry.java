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
import java.net.JarURLConnection;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.jar.JarFile;

public class ElementRegistry
{
    private static final String ELEMENTS_PKG      = "dev.fempolyhedron.fourditor.elements";
    private static final String ELEMENTS_PKG_PATH = ELEMENTS_PKG.replace('.', '/');

    public static Map<String, String> buildMap()
    {
        Map<String, String> map = new LinkedHashMap<>();

        // 1. Built-in descriptors from this jar / classpath
        scanClassLoader(ElementRegistry.class.getClassLoader(), map);

        // 2. Plugin descriptors from addons/ directory next to the jar
        File addonsDir = findAddonsDir();
        if (addonsDir != null && addonsDir.isDirectory())
        {
            File[] jars = addonsDir.listFiles(f -> f.getName().endsWith(".jar"));
            if (jars != null)
            {
                for (File jar : jars)
                {
                    loadAddon(jar, map);
                }
            }
        }

        return map;
    }

    // -------------------------------------------------------------------------

    private static void scanClassLoader(ClassLoader cl, Map<String, String> map)
    {
        try
        {
            URL pkgUrl = cl.getResource(ELEMENTS_PKG_PATH);
            if (pkgUrl == null) return;

            if ("jar".equals(pkgUrl.getProtocol()))
            {
                scanJar(pkgUrl, cl, map);
            }
            else
            {
                scanDirectory(pkgUrl, cl, map);
            }
        }
        catch (Exception e)
        {
            System.err.println("fourditor: warning: element registry scan failed: " + e.getMessage());
        }
    }

    private static void scanJar(URL pkgUrl, ClassLoader cl, Map<String, String> map) throws Exception
    {
        JarURLConnection conn = (JarURLConnection) pkgUrl.openConnection();
        try (JarFile jar = conn.getJarFile())
        {
            jar.stream()
               .filter(e ->  e.getName().startsWith(ELEMENTS_PKG_PATH + "/")
                          && e.getName().endsWith(".class")
                          && !e.getName().contains("$"))
               .forEach(e ->
               {
                   String simple = e.getName()
                       .substring(ELEMENTS_PKG_PATH.length() + 1, e.getName().length() - 6);
                   loadDescriptor(ELEMENTS_PKG + "." + simple, cl, map);
               });
        }
    }

    private static void scanDirectory(URL pkgUrl, ClassLoader cl, Map<String, String> map) throws Exception
    {
        File dir = new File(pkgUrl.toURI());
        File[] files = dir.listFiles(f -> f.getName().endsWith(".class") && !f.getName().contains("$"));
        if (files == null) return;
        for (File f : files)
        {
            String simple = f.getName().substring(0, f.getName().length() - 6);
            loadDescriptor(ELEMENTS_PKG + "." + simple, cl, map);
        }
    }

    private static void loadDescriptor(String className, ClassLoader cl, Map<String, String> map)
    {
        try
        {
            Class<?> cls = Class.forName(className, true, cl);
            if (!ElementDescriptor.class.isAssignableFrom(cls) || cls.isInterface()) return;
            ElementDescriptor d = (ElementDescriptor) cls.getDeclaredConstructor().newInstance();
            for (String tag : d.tags())
            {
                map.put(tag, d.cppType());
            }
        }
        catch (Exception ignored) {}
    }

    // -------------------------------------------------------------------------

    private static void loadAddon(File jarFile, Map<String, String> map)
    {
        try
        {
            URLClassLoader addonCl = new URLClassLoader(
                new URL[]{ jarFile.toURI().toURL() },
                ElementRegistry.class.getClassLoader()
            );

            try (JarFile jar = new JarFile(jarFile))
            {
                List<String> candidates = new ArrayList<>();
                jar.stream()
                   .filter(e ->  e.getName().endsWith(".class")
                              && !e.getName().contains("$"))
                   .forEach(e ->
                   {
                       String cls = e.getName().replace('/', '.').substring(0, e.getName().length() - 6);
                       candidates.add(cls);
                   });

                for (String cls : candidates)
                {
                    loadDescriptor(cls, addonCl, map);
                }
            }
        }
        catch (Exception e)
        {
            System.err.println("fourditor: warning: failed to load addon jar '"
                + jarFile.getName() + "': " + e.getMessage());
        }
    }

    private static File findAddonsDir()
    {
        try
        {
            URL loc = ElementRegistry.class.getProtectionDomain().getCodeSource().getLocation();
            File jarFile = new File(loc.toURI());
            // If running from a jar, addons/ is next to it; from a class dir, go up to project root
            File base = jarFile.isFile() ? jarFile.getParentFile() : jarFile.getParentFile();
            return new File(base, "addons");
        }
        catch (Exception e)
        {
            return null;
        }
    }
}
