package dev.fempolyhedron.fourditor.elements;
import dev.fempolyhedron.fourditor.ElementDescriptor;

public class Heading2Descriptor implements ElementDescriptor
{
    public String[] tags()  { return new String[]{"h2", "heading2"}; }
    public String cppType() { return "Fourtitude::Elements::Heading2"; }
}
