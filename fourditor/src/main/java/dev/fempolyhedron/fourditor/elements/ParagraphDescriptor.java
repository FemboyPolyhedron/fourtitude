package dev.fempolyhedron.fourditor.elements;
import dev.fempolyhedron.fourditor.ElementDescriptor;

public class ParagraphDescriptor implements ElementDescriptor
{
    public String[] tags()  { return new String[]{"p", "paragraph"}; }
    public String cppType() { return "Fourtitude::Elements::Paragraph"; }
}
