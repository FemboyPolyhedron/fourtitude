package dev.fempolyhedron.fourditor.elements;
import dev.fempolyhedron.fourditor.ElementDescriptor;

public class ContainerDescriptor implements ElementDescriptor
{
    public String[] tags()  { return new String[]{"container"}; }
    public String cppType() { return "Fourtitude::Elements::Container"; }
}
