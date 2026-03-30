package dev.fempolyhedron.fourditor.elements;
import dev.fempolyhedron.fourditor.ElementDescriptor;

public class InputDescriptor implements ElementDescriptor
{
    public String[] tags()  { return new String[]{"input"}; }
    public String cppType() { return "Fourtitude::Elements::Input"; }
}
