#include <ft2build.h>
#include FT_FREETYPE_H          // <freetype/freetype.h>
#include FT_MODULE_H            // <freetype/ftmodapi.h>
#include FT_GLYPH_H             // <freetype/ftglyph.h>
#include FT_SYNTHESIS_H         // <freetype/ftsynth.h>
#include FT_OTSVG_H             // <freetype/otsvg.h>
#include FT_BBOX_H              // <freetype/ftbbox.h>


#include <string>
#include <cassert>


#define ASSERT(_EXPR) assert(_EXPR)
// From SDL_ttf: Handy routines for converting from fixed point
#define FT_CEIL(X)  (((X + 63) & -64) / 64)

//
// Our Svg rendering hooks: Hook_PresetSlot is the interesting one here.
// It prints the svg_document_length
//
static FT_Error Hook_PresetSlot(FT_GlyphSlot slot, FT_Bool /*cache*/, FT_Pointer* /*_state*/)
{
    FT_SVG_Document   document = (FT_SVG_Document)slot->other;
    printf(" - Hook_PresetSlot: svg_document_length: %ld", document->svg_document_length);
    return FT_Err_Ok;
}
static FT_Error Hook_InitSvg(FT_Pointer* /*_state*/)
{
    printf("  - Hook_InitSvg");
    return FT_Err_Ok;
}
static void Hook_FreeSvg(FT_Pointer* /*_state*/)
{
    printf("Hook_FreeSvg\n");
}
static FT_Error Hook_RenderSvg(FT_GlyphSlot /*slot*/, FT_Pointer* /*_state*/)
{
    printf(" - Hook_RenderSvg");
    return FT_Err_Ok;
}


// Font parameters and metrics.
struct FontInfo
{
    uint32_t    PixelHeight = 0;          // Size this font was generated with.
    float       Ascender = 0.f;           // The pixel extents above the baseline in pixels (typically positive).
    float       Descender = 0.f;          // The extents below the baseline in pixels (typically negative).
    float       LineSpacing = 0.f;        // The baseline-to-baseline distance. Note that it is usually larger than the sum of the ascender and descender taken as absolute values. There is also no guarantee that no glyphs extend above or below subsequent baselines when using this distance. Think of it as a value the designer of the font finds appropriate.
    float       LineGap = 0.f;            // The spacing in pixels between one row's descent and the next row's ascent.
    float       MaxAdvanceWidth = 0.f;    // This field gives the maximum horizontal cursor advance for all glyphs in the font.
};
// A structure that describe a glyph.
struct GlyphInfo
{
    int         Width = 0;              // Glyph's width in pixels.
    int         Height = 0;             // Glyph's height in pixels.
    FT_Int      OffsetX = 0;            // The distance from the origin ("pen position") to the left of the glyph.
    FT_Int      OffsetY = 0;            // The distance from the origin to the top of the glyph. This is usually a value < 0.
    float       AdvanceX = 0.f;         // The distance from the origin to the origin of the next glyph. This is usually a value > 0.
    bool        IsColored = false;      // The glyph is colored
};


// A simple font loading utility, by no means complete or functional
struct FreetypeFont
{
    void InitFont(FT_Library ft_library, const char* fontPath, int SizePixels)
    {
        FT_Error error;
        int face_index = 0;
        error = FT_New_Face(ft_library, fontPath, face_index, &Face);
        ASSERT(error == 0);
        error = FT_Select_Charmap(Face, FT_ENCODING_UNICODE);
        ASSERT(error == 0);

        LoadFlags |= FT_LOAD_NO_BITMAP;
        LoadFlags |= FT_LOAD_TARGET_NORMAL;
        LoadFlags |= FT_LOAD_COLOR;
        RenderMode = FT_RENDER_MODE_NORMAL;

        SetPixelHeight(SizePixels);
    }

    void SetPixelHeight(int pixel_height)
    {
        FT_Size_RequestRec req;
        req.type = FT_SIZE_REQUEST_TYPE_REAL_DIM;
        req.width = 0;
        req.height = (uint32_t)(pixel_height * 64 * RasterizationDensity);
        req.horiResolution = 0;
        req.vertResolution = 0;
        FT_Request_Size(Face, &req);

        // Update font info
        FT_Size_Metrics metrics = Face->size->metrics;
        Info.PixelHeight = (uint32_t)(pixel_height * InvRasterizationDensity);
        Info.Ascender = (float)FT_CEIL(metrics.ascender) * InvRasterizationDensity;
        Info.Descender = (float)FT_CEIL(metrics.descender) * InvRasterizationDensity;
        Info.LineSpacing = (float)FT_CEIL(metrics.height) * InvRasterizationDensity;
        Info.LineGap = (float)FT_CEIL(metrics.height - metrics.ascender + metrics.descender) * InvRasterizationDensity;
        Info.MaxAdvanceWidth = (float)FT_CEIL(metrics.max_advance) * InvRasterizationDensity;
    }

    void CloseFont()
    {
        if (Face)
        {
            FT_Done_Face(Face);
            Face = nullptr;
        }
    }

    const FT_Glyph_Metrics* LoadGlyph(uint32_t codepoint)
    {
        uint32_t glyph_index = FT_Get_Char_Index(Face, codepoint);
        if (glyph_index == 0)
            return nullptr;

        FT_Error error = FT_Load_Glyph(Face, glyph_index, LoadFlags);
        if (error)
            return nullptr;

        // Need an outline for this to work
        FT_GlyphSlot slot = Face->glyph;

        ASSERT(slot->format == FT_GLYPH_FORMAT_OUTLINE || slot->format == FT_GLYPH_FORMAT_BITMAP || slot->format == FT_GLYPH_FORMAT_SVG);

        return &slot->metrics;
    }

    const FT_Bitmap* RenderGlyphAndGetInfo(GlyphInfo* out_glyph_info)
    {
        FT_GlyphSlot slot = Face->glyph;
        FT_Error error = FT_Render_Glyph(slot, RenderMode);
        if (error != 0)
            return nullptr;

        FT_Bitmap* ft_bitmap = &Face->glyph->bitmap;
        out_glyph_info->Width = (int)ft_bitmap->width;
        out_glyph_info->Height = (int)ft_bitmap->rows;
        out_glyph_info->OffsetX = Face->glyph->bitmap_left;
        out_glyph_info->OffsetY = -Face->glyph->bitmap_top;
        out_glyph_info->AdvanceX = (float)FT_CEIL(slot->advance.x);
        out_glyph_info->IsColored = (ft_bitmap->pixel_mode == FT_PIXEL_MODE_BGRA);

        return ft_bitmap;
    }

    void LoadGlyphs(FT_ULong codepoint_start, FT_ULong codepoint_end)
    {
        int nb_glyphs = 0;
        for (FT_ULong codepoint = codepoint_start; codepoint <= codepoint_end; codepoint++)
        {
            uint32_t glyph_index = FT_Get_Char_Index(Face, codepoint);
            if (glyph_index == 0)
                continue;

            printf("codepoint: %d glyph_index: %d  ", codepoint, glyph_index);

            ++nb_glyphs;
            const FT_Glyph_Metrics* metrics = LoadGlyph(codepoint);
            (void)metrics;
            
            // We could render the glyph here, but the issue is visible just by calling LoadGlyph,
            // which will call our hook Hook_PresetSlot
            // Below, ft_bitmap would be null since our hook only checks svg_document_length
            if (false)
            {
                 if (metrics == nullptr)
                    continue;
                 GlyphInfo glyphInfo;
                 const FT_Bitmap* ft_bitmap = RenderGlyphAndGetInfo(&glyphInfo);
            }

            printf("\n");
        }
        printf("nb_glyphs: %d\n", nb_glyphs);
    }


    FontInfo        Info;               // Font descriptor of the current font.
    FT_Face         Face = nullptr;
    FT_Int32        LoadFlags = 0;
    FT_Render_Mode  RenderMode = FT_RENDER_MODE_NORMAL;
    float           RasterizationDensity = 1.0f;
    float           InvRasterizationDensity = 1.0f / RasterizationDensity;

};


std::string ThisDir()
{
    std::string thisFile = __FILE__;
    size_t lastSlash = thisFile.find_last_of("/\\");
    if (lastSlash == std::string::npos) return ""; // No slashes found
    return thisFile.substr(0, lastSlash) + "/";
}


int main()
{
    FT_Library ft_library;
    FT_Error error;
    error = FT_Init_FreeType(&ft_library);
    ASSERT(error == 0);
    FT_Add_Default_Modules(ft_library);

    // Install svg hooks for FreeType
    SVG_RendererHooks hooks = {Hook_InitSvg, Hook_FreeSvg, Hook_RenderSvg, Hook_PresetSlot };
    FT_Property_Set(ft_library, "ot-svg", "svg-hooks", &hooks);

    std::string fontPath = ThisDir() + "fonts/NotoColorEmoji-Regular.ttf";
    FreetypeFont freetypeFont;
    freetypeFont.InitFont(ft_library, fontPath.c_str(), 30);
    freetypeFont.LoadGlyphs(0x1, 0x1FFFF);
    freetypeFont.CloseFont();

    FT_Done_FreeType(ft_library);
    return 0;
}
