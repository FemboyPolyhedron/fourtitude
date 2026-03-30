package dev.fempolyhedron.fourditor.elements;
import dev.fempolyhedron.fourditor.ElementDescriptor;

public class ListDescriptor implements ElementDescriptor
{
    public String[] tags()  { return new String[]{"list"}; }
    public String cppType() { return "Fourtitude::Elements::List"; }
}
