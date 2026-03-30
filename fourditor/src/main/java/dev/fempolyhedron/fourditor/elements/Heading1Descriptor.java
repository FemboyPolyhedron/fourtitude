package dev.fempolyhedron.fourditor.elements;
import dev.fempolyhedron.fourditor.ElementDescriptor;

public class Heading1Descriptor implements ElementDescriptor
{
    public String[] tags()  { return new String[]{"h1", "heading1"}; }
    public String cppType() { return "Fourtitude::Elements::Heading1"; }
}
