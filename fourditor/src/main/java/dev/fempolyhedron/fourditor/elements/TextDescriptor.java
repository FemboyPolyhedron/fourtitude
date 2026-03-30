package dev.fempolyhedron.fourditor.elements;
import dev.fempolyhedron.fourditor.ElementDescriptor;

public class TextDescriptor implements ElementDescriptor
{
    public String[] tags()  { return new String[]{"text"}; }
    public String cppType() { return "Fourtitude::Elements::Text"; }
}
