"""Export REPORT.md to REPORT.pdf. Install scripts/requirements.txt first."""
from pathlib import Path
import re
from xml.sax.saxutils import escape
import reportlab
import matplotlib
import pymupdf
from io import BytesIO
from reportlab.lib import colors
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.lib.enums import TA_LEFT
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, Image, PageBreak, Preformatted, KeepTogether

ROOT = Path(__file__).resolve().parents[1]
FONT_DIR = Path(matplotlib.get_data_path()) / 'fonts/ttf'
for name,file in [('Report','DejaVuSans.ttf'),('ReportBold','DejaVuSans-Bold.ttf'),('ReportItalic','DejaVuSans-Oblique.ttf')]:
    pdfmetrics.registerFont(TTFont(name,str(FONT_DIR/file)))
pdfmetrics.registerFontFamily('Report',normal='Report',bold='ReportBold',italic='ReportItalic',boldItalic='ReportBold')
styles=getSampleStyleSheet()
for style in styles.byName.values(): style.fontName='Report'
styles['BodyText'].fontSize=9
styles['BodyText'].leading=14
styles['BodyText'].spaceAfter=6
styles['Heading1'].fontName=styles['Heading2'].fontName='ReportBold'
styles['Heading2'].spaceAfter=14
styles.add(ParagraphStyle('Cell',fontName='Report',fontSize=7,leading=10,wordWrap='CJK'))
styles.add(ParagraphStyle('Caption',parent=styles['BodyText'],fontSize=8,leading=11,textColor=colors.HexColor('#475569')))


def inline(text):
    text=escape(text)
    def link(match):
        label, target = match.groups()
        if not target.startswith('https://'):
            target = 'https://github.com/MayankMoyal/DSA-Project/blob/44770556639c747bb4a11a83c60f65918004e677/' + target
        return f'<link href="{target}" color="#205078">{label}</link>'
    text=re.sub(r'\[([^]]+)\]\(([^)]+)\)',link,text)
    text=re.sub(r'`([^`]+)`',r'<font name="Report">\1</font>',text)
    text=re.sub(r'\*\*([^*]+)\*\*',r'<b>\1</b>',text)
    return text


def footer(canvas,doc):
    canvas.setFont('Report',8)
    canvas.setFillColor(colors.HexColor('#64748b'))
    canvas.drawString(42,25,'Underqualified | Project 3 | Report')
    canvas.drawRightString(553,25,str(doc.page))


def main():
    lines=(ROOT/'REPORT.md').read_text(encoding='utf-8-sig').splitlines()
    story=[]; i=0; section=False
    while i<len(lines):
        line=lines[i].strip()
        if not line: i+=1; continue
        if re.match(r'^## \d+\.',line):
            if section: story.append(Spacer(1,16))
            section=True
            story.append(Paragraph(inline(line[3:]),styles['Heading2'])); i+=1; continue
        if line.startswith('#'):
            level=len(line)-len(line.lstrip('#'))
            story.append(Paragraph(inline(line.lstrip('# ')),styles['Heading1' if level==1 else 'Heading3'])); i+=1; continue
        match=re.fullmatch(r'!\[([^]]*)\]\(([^)]+)\)',line)
        if match:
            source = ROOT / match[2]
            if source.suffix.lower() == '.svg':
                with pymupdf.open(source) as vector:
                    pixels = vector[0].get_pixmap(matrix=pymupdf.Matrix(2, 2), alpha=False)
                    image = Image(BytesIO(pixels.tobytes("png")))
            else:
                image = Image(str(source))
            factor=min(510/image.imageWidth,248/image.imageHeight)
            image.drawWidth=image.imageWidth*factor; image.drawHeight=image.imageHeight*factor
            story.extend([KeepTogether([image, Paragraph(inline(match[1]), styles['Caption'])]), Spacer(1,8)])
            i+=1; continue
        if line.startswith('|'):
            rows=[]
            while i<len(lines) and lines[i].strip().startswith('|'):
                cells=[x.strip() for x in lines[i].strip().strip('|').split('|')]
                if not all(re.fullmatch(r'[:\- ]+',x) for x in cells):
                    rows.append([Paragraph(inline(c),styles['Cell']) for c in cells])
                i+=1
            table=Table(rows,colWidths=[510/len(rows[0])]*len(rows[0]),repeatRows=1,hAlign='LEFT')
            table.setStyle(TableStyle([('BACKGROUND',(0,0),(-1,0),colors.HexColor('#e2e8f0')),('VALIGN',(0,0),(-1,-1),'TOP'),('GRID',(0,0),(-1,-1),.4,colors.HexColor('#cbd5e1')),('TOPPADDING',(0,0),(-1,-1),6),('BOTTOMPADDING',(0,0),(-1,-1),6)]))
            story.extend([KeepTogether([table]),Spacer(1,12)]); continue
        if line.startswith('- '):
            story.append(Paragraph(inline(line[2:]),styles['BodyText'],bulletText='•')); i+=1; continue
        paragraph=[line]; i+=1
        while i<len(lines) and lines[i].strip() and not lines[i].startswith(('#','|','![','- ')):
            paragraph.append(lines[i].strip()); i+=1
        story.append(Paragraph(inline(' '.join(paragraph)),styles['BodyText']))
    doc=SimpleDocTemplate(str(ROOT/'REPORT.pdf'),pagesize=(595,842),leftMargin=42,rightMargin=43,topMargin=40,bottomMargin=42,title='Underqualified: Search Trees Under Long-Running Churn',author='Team Underqualified')
    doc.build(story,onFirstPage=footer,onLaterPages=footer)
    print('Exported REPORT.pdf from REPORT.md')


if __name__=='__main__': main()
