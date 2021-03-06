// DirectXTK MakeSpriteFont tool
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// http://go.microsoft.com/fwlink/?LinkId=248929

using System;
using System.IO;
using System.Linq;
using System.Drawing;

namespace MakeSpriteFont
{
    public class Program
    {
        public static int Main(string[] args)
        {
            // Parse the commandline options.
            var options = new CommandLineOptions();
            var parser = new CommandLineParser(options);

            if (!parser.ParseCommandLine(args))
                return 1;

            try
            {
                // Convert the font.
                MakeSpriteFont(options);
                
                return 0;
            }
            catch (Exception e)
            {
                // Print an error message if conversion failed.
                Console.WriteLine();
                Console.Error.WriteLine("Error: {0}", e.Message);
     
                return 1;
            }
        }


        static void MakeSpriteFont(CommandLineOptions options)
        {
            // Import.
            Console.WriteLine("Importing {0}", options.SourceFont);

            float lineSpacing;

            Glyph[] glyphs = ImportFont(options, out lineSpacing);

            // Optimize.
            Console.WriteLine("Cropping glyph borders");

            foreach (Glyph glyph in glyphs)
            {
                GlyphCropper.Crop(glyph);
            }

            Console.WriteLine("Packing glyphs into sprite sheet");

            Bitmap bitmap = GlyphPacker.ArrangeGlyphs(glyphs);

            // Adjust line and character spacing.
            lineSpacing += options.LineSpacing;

            foreach (Glyph glyph in glyphs)
            {
                glyph.XAdvance += options.CharacterSpacing;
            }

            // Automatically detect whether this is a monochromatic or color font?
            if (options.TextureFormat == TextureFormat.Auto)
            {
                bool isMono = BitmapUtils.IsRgbEntirely(Color.White, bitmap);

                options.TextureFormat = isMono ? TextureFormat.CompressedMono :
                                                 TextureFormat.Rgba32;
            }

            // Convert to premultiplied alpha format.
            if (!options.NoPremultiply)
            {
                Console.WriteLine("Premultiplying alpha");

                BitmapUtils.PremultiplyAlpha(bitmap);
            }

            // Save output files.
            if (!string.IsNullOrEmpty(options.DebugOutputSpriteSheet))
            {
                Console.WriteLine("Saving debug output spritesheet {0}", options.DebugOutputSpriteSheet);

                bitmap.Save(options.DebugOutputSpriteSheet);
            }

            Console.WriteLine("Writing {0} ({1} format)", options.OutputFile, options.TextureFormat);

            SpriteFontWriter.WriteSpriteFont(options, glyphs, lineSpacing, bitmap);
        }


        static Glyph[] ImportFont(CommandLineOptions options, out float lineSpacing)
        {
            // Which importer knows how to read this source font?
            IFontImporter importer;

            string fileExtension = Path.GetExtension(options.SourceFont).ToLowerInvariant();

            string[] BitmapFileExtensions = { ".bmp", ".png", ".gif" };

            if (BitmapFileExtensions.Contains(fileExtension))
            {
                importer = new BitmapImporter();
            }
            else
            {
                importer = new TrueTypeImporter();
            }

            // Import the source font data.
            importer.Import(options);

            lineSpacing = importer.LineSpacing;

            var glyphs = importer.Glyphs
                                 .OrderBy(glyph => glyph.Character)
                                 .ToArray();

            // Validate.
            if (glyphs.Length == 0)
            {
                throw new Exception("Font does not contain any glyphs.");
            }

            if ((options.DefaultCharacter != 0) && !glyphs.Any(glyph => glyph.Character == options.DefaultCharacter))
            {
                throw new Exception("The specified DefaultCharacter is not part of this font.");
            }

            return glyphs;
        }
    }
}
