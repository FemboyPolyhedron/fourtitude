package dev.fempolyhedron.fourditor.elements;
import dev.fempolyhedron.fourditor.ElementDescriptor;

public class Heading3Descriptor implements ElementDescriptor
{
    public String[] tags()  { return new String[]{"h3", "heading3"}; }
    public String cppType() { return "Fourtitude::Elements::Heading3"; }
}
