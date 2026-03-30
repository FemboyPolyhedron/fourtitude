package dev.fempolyhedron.fourditor.elements;
import dev.fempolyhedron.fourditor.ElementDescriptor;

public class ButtonDescriptor implements ElementDescriptor
{
    public String[] tags()  { return new String[]{"button"}; }
    public String cppType() { return "Fourtitude::Elements::Button"; }
}
