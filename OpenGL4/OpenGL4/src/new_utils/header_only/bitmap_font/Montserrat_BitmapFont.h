#pragma once
#include "bitmap_font_base.h"

//Font is based on google's royality free front https://fonts.google.com/specimen/Montserrat

namespace ho
{
	class Montserrat_BMF : public BitmapFont
	{
	public:
		Montserrat_BMF(const Montserrat_BMF& copy) = delete;
		Montserrat_BMF(Montserrat_BMF&& move) = delete;
		Montserrat_BMF& operator=(const Montserrat_BMF& copy) = delete;
		Montserrat_BMF& operator=(Montserrat_BMF&& move) = delete;
		Montserrat_BMF(const char* texture_file_path)
			: BitmapFont(texture_file_path)
		{
			_configureGlyphTable();
		}

		void _configureGlyphTable()
		{
			if (_configured)
			{
				std::cerr << "_configureGlyphTable called, but table already configured" << std::endl;
				return;
			}
			_configured = true;

			//steps to mapping characters:
			//1. align bottom left corner
			//2. align width
			//3. align height
			//4. align baseline offset (for letters like g, p, q, where part of letter goes below baseline)
			//starting point: new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.78f, 0.965f), 0.025f, 0.025f); 

			//this method should only ever be called once
			glyphTable['a'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.7835f, 0.966f), 0.0155f, 0.0169f);
			glyphTable['b'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.806f, 0.966f), 0.018f, 0.02275f);
			glyphTable['c'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.0122f, 0.927f), 0.0165f, 0.0169f);
			glyphTable['d'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.074f, 0.927f), 0.0182f, 0.02275f);
			glyphTable['e'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.1215f, 0.927f), 0.0165f, 0.0169f);
			glyphTable['f'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.1420f, 0.927f), 0.0128f, 0.0234f);
			glyphTable['g'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.157f, 0.921f), 0.0174f, 0.0234f, -0.007f);
			glyphTable['h'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.1823f, 0.927f), 0.0167f, 0.0234f);
			glyphTable['i'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.205f, 0.927f), 0.006f, 0.0234f);
			glyphTable['j'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.213f, 0.921f), 0.010f, 0.0287f, -0.006f);
			glyphTable['k'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.230f, 0.927f), 0.0167f, 0.0230f);
			glyphTable['l'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.2515f, 0.927f), 0.004f, 0.023f);
			glyphTable['m'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.263f, 0.927f), 0.028f, 0.017f);
			glyphTable['n'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.2985f, 0.927f), 0.0158f, 0.017f);
			glyphTable['o'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.3205f, 0.927f), 0.0177f, 0.017f);
			glyphTable['p'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.3443f, 0.921f), 0.0175f, 0.0235f, -0.006f);
			glyphTable['q'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.3667f, 0.921f), 0.0175f, 0.0235f, -0.006f);
			glyphTable['r'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.3922f, 0.927f), 0.010f, 0.017f);
			glyphTable['s'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.4058f, 0.927f), 0.014f, 0.017f);
			glyphTable['t'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.442f, 0.927f), 0.0115f, 0.021f);
			glyphTable['u'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.4595f, 0.927f), 0.016f, 0.017f);
			glyphTable['v'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.481f, 0.927f), 0.017f, 0.017f);
			glyphTable['w'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.501f, 0.927f), 0.0268f, 0.017f);
			glyphTable['x'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.5307f, 0.927f), 0.016f, 0.017f);
			glyphTable['y'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.5485f, 0.921f), 0.0184f, 0.0287f, -0.006f);
			glyphTable['z'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.5707f, 0.927f), 0.014f, 0.017f);
			glyphTable['A'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.01125f, 0.966f), 0.0227f, 0.0215f);
			glyphTable['B'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.0395f, 0.966f), 0.0195f, 0.0215f);
			glyphTable['C'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.0641f, 0.966f), 0.0195f, 0.02155f);
			glyphTable['D'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.14125f, 0.966f), 0.0205f, 0.02155f);
			glyphTable['E'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.1965f, 0.966f), 0.017f, 0.0218f);
			glyphTable['F'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.2202f, 0.966f), 0.017f, 0.0218f);
			glyphTable['G'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.241f, 0.966f), 0.0205f, 0.0218f);
			glyphTable['H'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.27f, 0.966f), 0.0185f, 0.0218f);
			glyphTable['I'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.2975f, 0.966f), 0.0038f, 0.0218f);
			glyphTable['J'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.3065f, 0.966f), 0.0134f, 0.0218f);
			glyphTable['K'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.329f, 0.966f), 0.018f, 0.0218f);
			glyphTable['L'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.353f, 0.966f), 0.016f, 0.0218f);
			glyphTable['M'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.3745f, 0.966f), 0.02325f, 0.0218f);
			glyphTable['N'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.4065f, 0.966f), 0.019f, 0.0218f);
			glyphTable['O'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.432f, 0.966f), 0.024f, 0.0218f);
			glyphTable['P'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.4625f, 0.966f), 0.018f, 0.0218f);
			glyphTable['Q'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.4859f, 0.962f), 0.024f, 0.0260f, -0.0042f);
			glyphTable['R'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.51625f, 0.966f), 0.018f, 0.0218f);
			glyphTable['S'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.5395f, 0.966f), 0.0167f, 0.0218f);
			glyphTable['T'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.5817f, 0.966f), 0.0185f, 0.0218f);
			glyphTable['U'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.606f, 0.966f), 0.0184f, 0.0218f);
			glyphTable['V'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.6295f, 0.966f), 0.0221f, 0.0218f);
			glyphTable['W'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.655f, 0.966f), 0.0323f, 0.0218f);
			glyphTable['X'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.691f, 0.966f), 0.02f, 0.0218f);
			glyphTable['Y'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.7136f, 0.966f), 0.02f, 0.0218f);
			glyphTable['Z'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.737f, 0.966f), 0.019f, 0.0218f);
			//numeric row
			glyphTable['0'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.057f, 0.771f), 0.019f, 0.0218f);
			glyphTable['1'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.683f, 0.81f), 0.0088f, 0.0218f);
			glyphTable['2'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.698f, 0.81f), 0.017f, 0.0218f);
			glyphTable['3'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.7182f, 0.81f), 0.0166f, 0.0218f);
			glyphTable['4'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.7395f, 0.81f), 0.02f, 0.0218f);
			glyphTable['5'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.7625f, 0.81f), 0.0165f, 0.0218f);
			glyphTable['6'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.7838f, 0.81f), 0.017f, 0.0218f);
			glyphTable['7'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.805f, 0.81f), 0.017f, 0.0218f);
			glyphTable['8'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.0132f, 0.771f), 0.0172f, 0.0218f);
			glyphTable['9'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.0352f, 0.771f), 0.017f, 0.0218f);
			glyphTable['\''] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.08f, 0.771f), 0.006f, 0.0225f);
			glyphTable['?'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.09f, 0.771f), 0.016f, 0.0218f);
			glyphTable['\"'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.121f, 0.771f), 0.0105f, 0.0225f);
			glyphTable['!'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.137f, 0.771f), 0.005f, 0.0218f);
			glyphTable['('] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.164f, 0.765f), 0.008f, 0.0285f, -0.003f);  //dropped
			glyphTable['%'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.1764f, 0.771f), 0.024f, 0.0218f);
			glyphTable[')'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.204f, 0.765f), 0.008f, 0.0285f, -0.003f); //dropped
			glyphTable['['] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.22f, 0.765f), 0.008f, 0.0285f, -0.003f); //dropped
			glyphTable['#'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.23075f, 0.771f), 0.021f, 0.0218f);
			glyphTable[']'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.255f, 0.765f), 0.008f, 0.0285f, -0.003f);//dropped
			glyphTable['{'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.269f, 0.765f), 0.01f, 0.0285f, -0.003f); //dropped
			glyphTable['@'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.2825f, 0.765f), 0.0295f, 0.0285f, -0.006f); //dropped
			glyphTable['}'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.3155f, 0.765f), 0.0105f, 0.0285f, -0.003f); //dropped

			//start here
			glyphTable['/'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.3288f, 0.768f), 0.014f, 0.029f, -0.003f); //slight drop
			glyphTable['&'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.343f, 0.771f), 0.0206f, 0.0218f);
			glyphTable['\\'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.365f, 0.768f), 0.014f, 0.029f, -0.003f); //slight drop
			glyphTable['<'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.3825f, 0.771f), 0.015f, 0.018f);
			glyphTable['-'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.4028f, 0.771f), 0.01f, 0.01f);
			glyphTable['+'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.4185f, 0.771f), 0.0145f, 0.0181f);
			glyphTable['÷'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.439f, 0.771f), 0.0146f, 0.0182f);
			glyphTable['='] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.4808f, 0.771f), 0.0145f, 0.017f);
			glyphTable['>'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.501f, 0.771f), 0.015f, 0.018f);

			//glyphTable['®'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.521f, 0.771f), 0.0227f, 0.0218f);
			glyphTable['©'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.549f, 0.771f), 0.0224f, 0.0218f);
			glyphTable['$'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.5765f, 0.767f), 0.0168f, 0.0295f, -0.0035f); //dropped
			glyphTable[':'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.693f, 0.771f), 0.005f, 0.017f);
			glyphTable[';'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.7035f, 0.7674f), 0.005f, 0.02f, -0.0037f); //dropped
			glyphTable[','] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.7135f, 0.7674f), 0.005f, 0.009f, -0.0045f); //droped
			glyphTable['.'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.724f, 0.771f), 0.005f, 0.004f);
			glyphTable['*'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.733f, 0.771f), 0.012f, 0.023f);

			glyphTable['^'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.424f, 0.832f), 0.011f, 0.006f, 0.018f); //raised
			glyphTable['_'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.5817f, 0.9852f), 0.0185f, 0.0218f);
			glyphTable['|'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.2595f, 0.765f), 0.002f, 0.0285f, -0.003f); //drop like []
			glyphTable['`'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.059f, 0.944f), 0.009f, 0.006f, 0.02f); //flip?
			glyphTable['~'] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.5907f, 0.944f), 0.012f, 0.006f, 0.01f); //raised

			glyphTable[' '] = new_sp<GlyphRenderer>( shader, fontTexture, glm::vec2(0.5907f, 0.954f), 0.012f, 0.006f);

			//accents
			//glyphTable['ç'] = sp<GlyphRenderer>(nullptr);
			//glyphTable['â'] = sp<GlyphRenderer>(nullptr);
			//glyphTable['à'] = sp<GlyphRenderer>(nullptr);
			//glyphTable['é'] = sp<GlyphRenderer>(nullptr);
			//glyphTable['è'] = sp<GlyphRenderer>(nullptr);
			//glyphTable['ê'] = sp<GlyphRenderer>(nullptr);
			//glyphTable['ë'] = sp<GlyphRenderer>(nullptr);
			//glyphTable['î'] = sp<GlyphRenderer>(nullptr);
			//glyphTable['ï'] = sp<GlyphRenderer>(nullptr);
			//glyphTable['ô'] = sp<GlyphRenderer>(nullptr);
			//glyphTable['û'] = sp<GlyphRenderer>(nullptr);
			//glyphTable['ù'] = sp<GlyphRenderer>(nullptr);
			//glyphTable['ü'] = sp<GlyphRenderer>(nullptr);
			//there exists more accent than these

			//make glyphs self-aware of the charactesr the glyph represents.
			for (auto kv_pair : glyphTable)
			{
				const sp<GlyphRenderer>& glyph = kv_pair.second;
				if (glyph)
				{
					glyph->setKey(kv_pair.first);
				}
			}
		}
		private:
			bool _configured = false;
	};
}