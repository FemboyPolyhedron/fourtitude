package dev.fempolyhedron.fourditor.elements;
import dev.fempolyhedron.fourditor.ElementDescriptor;

public class FrameDescriptor implements ElementDescriptor
{
    public String[] tags()  { return new String[]{"frame"}; }
    public String cppType() { return "Fourtitude::Elements::Frame"; }
}
