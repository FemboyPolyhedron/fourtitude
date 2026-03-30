package dev.fempolyhedron.fourditor.elements;
import dev.fempolyhedron.fourditor.ElementDescriptor;

public class ImageDescriptor implements ElementDescriptor
{
    public String[] tags()  { return new String[]{"image", "img"}; }
    public String cppType() { return "Fourtitude::Elements::Image"; }
}
