const pptxgen = require("pptxgenjs");

const pres = new pptxgen();
pres.layout = "LAYOUT_16x9";
pres.title = "Environment & Object Modelling";

// ── Palette ──────────────────────────────────────────────
const BG        = "0D1B2A";   // deep navy
const BG_LIGHT  = "112233";   // slightly lighter navy for cards
const ACCENT    = "00B4D8";   // vivid teal
const ACCENT2   = "0096C7";   // deeper teal
const WHITE     = "E8F4FD";   // near-white text
const MUTED     = "8BAFC4";   // muted blue-grey
const GOLD      = "FFD166";   // warm gold for highlights
const DARK_CARD = "0A1628";   // card background

// ── Helper: shadow factory ────────────────────────────────
const mkShadow = () => ({ type: "outer", color: "000000", blur: 8, offset: 3, angle: 135, opacity: 0.35 });

// ── Helper: accent left bar ───────────────────────────────
function accentBar(slide, y, h, color = ACCENT) {
  slide.addShape(pres.shapes.RECTANGLE, {
    x: 0.45, y, w: 0.07, h,
    fill: { color },
    line: { color, width: 0 }
  });
}

// ── Helper: section title ─────────────────────────────────
function sectionTitle(slide, text, y = 0.38) {
  slide.addText(text, {
    x: 0.62, y, w: 9.2, h: 0.55,
    fontSize: 28, bold: true,
    fontFace: "Trebuchet MS",
    color: WHITE,
    margin: 0
  });
}

// ── Helper: subtitle label (gold badge) ──────────────────
function badge(slide, text, x, y) {
  slide.addShape(pres.shapes.RECTANGLE, {
    x, y, w: text.length * 0.095 + 0.25, h: 0.28,
    fill: { color: ACCENT2 },
    line: { color: ACCENT2, width: 0 },
    shadow: mkShadow()
  });
  slide.addText(text, {
    x, y, w: text.length * 0.095 + 0.25, h: 0.28,
    fontSize: 9, bold: true, fontFace: "Trebuchet MS",
    color: "FFFFFF", align: "center", valign: "middle", margin: 0
  });
}

// ── Helper: card background ───────────────────────────────
function card(slide, x, y, w, h) {
  slide.addShape(pres.shapes.RECTANGLE, {
    x, y, w, h,
    fill: { color: DARK_CARD },
    line: { color: ACCENT, width: 0.8 },
    shadow: mkShadow()
  });
}

// ── Helper: bullet block ──────────────────────────────────
function bulletBlock(slide, items, x, y, w, h, fontSize = 13) {
  const runs = [];
  items.forEach((item, i) => {
    const isLast = i === items.length - 1;
    if (item.label) {
      runs.push({ text: item.label + " ", options: { bold: true, color: GOLD, fontSize, fontFace: "Calibri", breakLine: false } });
      runs.push({ text: item.text, options: { bold: false, color: WHITE, fontSize, fontFace: "Calibri", breakLine: !isLast } });
    } else {
      runs.push({ text: item.text, options: { bullet: { indent: 14 }, color: WHITE, fontSize, fontFace: "Calibri", bold: false, breakLine: !isLast } });
    }
  });
  slide.addText(runs, { x, y, w, h, valign: "top", margin: [6, 8, 6, 8], paraSpaceAfter: 4 });
}

// ═══════════════════════════════════════════════════════════
// SLIDE 1 — Title
// ═══════════════════════════════════════════════════════════
{
  const s = pres.addSlide();
  s.background = { color: BG };

  // Top accent band
  s.addShape(pres.shapes.RECTANGLE, { x: 0, y: 0, w: 10, h: 0.08, fill: { color: ACCENT }, line: { color: ACCENT, width: 0 } });
  // Bottom accent band
  s.addShape(pres.shapes.RECTANGLE, { x: 0, y: 5.545, w: 10, h: 0.08, fill: { color: ACCENT }, line: { color: ACCENT, width: 0 } });

  // Decorative corner squares
  for (const [cx, cy] of [[0.3, 0.8], [9.5, 0.8], [0.3, 4.6], [9.5, 4.6]]) {
    s.addShape(pres.shapes.RECTANGLE, { x: cx, y: cy, w: 0.15, h: 0.15, fill: { color: ACCENT }, line: { color: ACCENT, width: 0 } });
  }

  // Vertical side lines
  s.addShape(pres.shapes.LINE, { x: 0.55, y: 0.9, w: 0, h: 3.8, line: { color: ACCENT, width: 1.2, dashType: "dash" } });
  s.addShape(pres.shapes.LINE, { x: 9.45, y: 0.9, w: 0, h: 3.8, line: { color: ACCENT, width: 1.2, dashType: "dash" } });

  // Main title
  s.addText("Environment &\nObject Modelling", {
    x: 1.0, y: 1.3, w: 8, h: 2.2,
    fontSize: 44, bold: true, fontFace: "Trebuchet MS",
    color: WHITE, align: "center", valign: "middle",
    shadow: mkShadow()
  });

  // Accent underline
  s.addShape(pres.shapes.RECTANGLE, { x: 3.2, y: 3.42, w: 3.6, h: 0.05, fill: { color: ACCENT }, line: { color: ACCENT, width: 0 } });

  // Subtitle
  s.addText("Tower Defense Balloon Shooter  ·  OpenGL", {
    x: 1, y: 3.65, w: 8, h: 0.5,
    fontSize: 16, fontFace: "Calibri", color: MUTED, align: "center"
  });

  // Bottom label
  s.addText("Computer Graphics Project", {
    x: 1, y: 5.05, w: 8, h: 0.35,
    fontSize: 11, fontFace: "Calibri", color: MUTED, align: "center", italic: true
  });
}

// ═══════════════════════════════════════════════════════════
// SLIDE 2 — Overview: Primitives
// ═══════════════════════════════════════════════════════════
{
  const s = pres.addSlide();
  s.background = { color: BG };
  s.addShape(pres.shapes.RECTANGLE, { x: 0, y: 0, w: 10, h: 0.08, fill: { color: ACCENT }, line: { color: ACCENT, width: 0 } });

  accentBar(s, 0.32, 0.62);
  sectionTitle(s, "Geometric Primitives — Building Blocks");

  // Three cards
  const primitives = [
    { title: "Cube", sub: "Most Common", color: ACCENT,
      lines: ["24 vertices, 36 indices (12 triangles)", "Unit cube −0.5 to 0.5 per axis", "Position + Normal + TexCoord (8 floats/vertex)", "Shared single VAO across entire scene"] },
    { title: "Sphere", sub: "Organic Shapes", color: GOLD,
      lines: ["Procedural: 36 sectors × 18 stacks", "Spherical coordinate generation", "Used for: balloons, sun/moon, leaves, clouds", "Dedicated sphereVAO with EBO indices"] },
    { title: "Cone", sub: "Decorative", color: "FF6B6B",
      lines: ["Procedural: 36 sectors, tapered sides", "Circular base cap included", "Dedicated coneVAO with EBO indices", "Used for ornamental scene elements"] },
  ];

  primitives.forEach((p, i) => {
    const x = 0.35 + i * 3.1;
    card(s, x, 1.15, 2.9, 4.1);
    // Top color band on card
    s.addShape(pres.shapes.RECTANGLE, { x, y: 1.15, w: 2.9, h: 0.35, fill: { color: p.color }, line: { color: p.color, width: 0 } });
    s.addText(p.title, { x: x + 0.1, y: 1.17, w: 2.0, h: 0.30, fontSize: 14, bold: true, fontFace: "Trebuchet MS", color: "FFFFFF", margin: 0 });
    s.addText(p.sub,   { x: x + 0.1, y: 1.48, w: 2.7, h: 0.22, fontSize: 10, fontFace: "Calibri", color: MUTED, margin: 0, italic: true });
    const runs = p.lines.map((l, li) => ({
      text: l,
      options: { bullet: { indent: 12 }, color: WHITE, fontSize: 11, fontFace: "Calibri", breakLine: li < p.lines.length - 1 }
    }));
    s.addText(runs, { x: x + 0.1, y: 1.75, w: 2.7, h: 3.3, valign: "top", margin: [4, 4, 4, 4], paraSpaceAfter: 5 });
  });

  // Bottom note
  s.addText("All objects are positioned and shaped via model matrix transforms: Translate · Rotate · Scale", {
    x: 0.45, y: 5.25, w: 9.1, h: 0.28,
    fontSize: 10.5, fontFace: "Calibri", color: MUTED, align: "center", italic: true
  });
}

// ═══════════════════════════════════════════════════════════
// SLIDE 3 — Ground & Road
// ═══════════════════════════════════════════════════════════
{
  const s = pres.addSlide();
  s.background = { color: BG };
  s.addShape(pres.shapes.RECTANGLE, { x: 0, y: 0, w: 10, h: 0.08, fill: { color: ACCENT }, line: { color: ACCENT, width: 0 } });

  accentBar(s, 0.32, 0.62);
  sectionTitle(s, "Ground & Road");
  badge(s, "ENVIRONMENT", 0.62, 1.05);

  // Two side-by-side cards
  const items = [
    {
      title: "Ground Plane", color: "2A9D8F",
      pts: [
        { label: "Primitive:", text: "Single cube, scaled (150 × 1 × 180)" },
        { label: "Texture:", text: "Grass texture tiled at scale 16× — GL_REPLACE mode" },
        { label: "Purpose:", text: "Forms the main island surface in world space" },
        { label: "Color:", text: "Green tint (0.2, 0.8, 0.2) blended with texture" },
      ]
    },
    {
      title: "Road (Balloon Path)", color: "E9C46A",
      pts: [
        { label: "Primitive:", text: "Flat cube, scaled (8 × 0.1 × 90)" },
        { label: "Texture:", text: "Asphalt texture tiled at scale 9× — GL_REPLACE mode" },
        { label: "Purpose:", text: "Defines the balloon travel corridor through the scene" },
        { label: "Placement:", text: "Rests on top of ground at y = −0.95" },
      ]
    }
  ];

  items.forEach((item, i) => {
    const x = 0.35 + i * 4.75;
    card(s, x, 1.42, 4.5, 3.9);
    s.addShape(pres.shapes.RECTANGLE, { x, y: 1.42, w: 4.5, h: 0.38, fill: { color: item.color }, line: { color: item.color, width: 0 } });
    s.addText(item.title, { x: x + 0.15, y: 1.44, w: 4.2, h: 0.33, fontSize: 15, bold: true, fontFace: "Trebuchet MS", color: "FFFFFF", margin: 0 });
    const runs = [];
    item.pts.forEach((p, pi) => {
      runs.push({ text: p.label + " ", options: { bold: true, color: GOLD, fontSize: 12.5, fontFace: "Calibri" } });
      runs.push({ text: p.text, options: { color: WHITE, fontSize: 12.5, fontFace: "Calibri", breakLine: pi < item.pts.length - 1 } });
    });
    s.addText(runs, { x: x + 0.15, y: 1.9, w: 4.2, h: 3.2, valign: "top", margin: [4, 6, 4, 6], paraSpaceAfter: 8 });
  });
}

// ═══════════════════════════════════════════════════════════
// SLIDE 4 — Balloon
// ═══════════════════════════════════════════════════════════
{
  const s = pres.addSlide();
  s.background = { color: BG };
  s.addShape(pres.shapes.RECTANGLE, { x: 0, y: 0, w: 10, h: 0.08, fill: { color: ACCENT }, line: { color: ACCENT, width: 0 } });

  accentBar(s, 0.32, 0.62);
  sectionTitle(s, "Balloon — Enemy Object");
  badge(s, "ENEMY", 0.62, 1.05);

  // Main card
  card(s, 0.35, 1.42, 9.3, 3.95);

  const parts = [
    { icon: "BODY",    color: "4361EE", label: "Body (Sphere)",  text: "A sphere scaled non-uniformly (0.6 × 0.8 × 0.6) producing an oval silhouette; colored blue with a procedural GPU shader pattern." },
    { icon: "STRING",  color: "7B2D8B", label: "String (Cube)",   text: "A very thin cube (0.05 × 0.8 × 0.05) parented below the balloon body to form the string." },
    { icon: "POP",     color: "EF233C", label: "Pop Effect",      text: "On hit, a scale multiplier grows over 0.15 s then resets; fragment cubes fly outward with individual velocity vectors." },
    { icon: "DATA",    color: "06A77D", label: "Struct Data",     text: "Stores position, color, scale, speed, active flag, popTimer, and fragment arrays in a C++ struct per balloon instance." },
  ];

  parts.forEach((p, i) => {
    const row = i;
    const y = 1.55 + row * 0.88;

    // Icon badge
    s.addShape(pres.shapes.RECTANGLE, { x: 0.55, y: y + 0.04, w: 0.65, h: 0.58, fill: { color: p.color }, line: { color: p.color, width: 0 }, shadow: mkShadow() });
    s.addText(p.icon, { x: 0.55, y: y + 0.04, w: 0.65, h: 0.58, fontSize: 7.5, bold: true, fontFace: "Trebuchet MS", color: "FFFFFF", align: "center", valign: "middle", margin: 0 });

    // Label + text
    s.addText(p.label, { x: 1.35, y: y + 0.05, w: 2.1, h: 0.28, fontSize: 13, bold: true, fontFace: "Trebuchet MS", color: GOLD, margin: 0 });
    s.addText(p.text,  { x: 1.35, y: y + 0.3,  w: 8.1, h: 0.52, fontSize: 12, fontFace: "Calibri", color: WHITE, margin: 0 });

    // Separator (not after last)
    if (i < parts.length - 1) {
      s.addShape(pres.shapes.LINE, { x: 0.55, y: y + 0.75, w: 8.9, h: 0, line: { color: "1A3550", width: 0.8 } });
    }
  });
}

// ═══════════════════════════════════════════════════════════
// SLIDE 5 — Tower
// ═══════════════════════════════════════════════════════════
{
  const s = pres.addSlide();
  s.background = { color: BG };
  s.addShape(pres.shapes.RECTANGLE, { x: 0, y: 0, w: 10, h: 0.08, fill: { color: ACCENT }, line: { color: ACCENT, width: 0 } });

  accentBar(s, 0.32, 0.62);
  sectionTitle(s, "Tower — Defense Structure");
  badge(s, "STRUCTURE", 0.62, 1.05);

  const parts = [
    { num: "01", color: "5C6BC0", label: "Base",     text: "Tall brick-textured cube scaled (2 × 8 × 2) — forms the tower shaft rising from the ground." },
    { num: "02", color: "26A69A", label: "Platform", text: "Wide flat cube scaled (4 × 0.2 × 4) placed at the top (y = 8.0) — the shooting and standing deck." },
    { num: "03", color: "EF9A9A", label: "Railings", text: "Four small posts (0.2 × 1.0 × 0.2) at platform corners — assembled from scaled cubes for decorative guard rails." },
    { num: "04", color: "FFCA28", label: "Placement",text: "Center at (0, −1.5, 0) world space; all active weapons fire from the platform level." },
  ];

  parts.forEach((p, i) => {
    const x = i < 2 ? 0.4 : 5.1;
    const y = i % 2 === 0 ? 1.52 : 3.35;
    card(s, x, y, 4.5, 1.6);
    // Number badge
    s.addShape(pres.shapes.RECTANGLE, { x: x + 0.1, y: y + 0.1, w: 0.5, h: 0.5, fill: { color: p.color }, line: { color: p.color, width: 0 } });
    s.addText(p.num, { x: x + 0.1, y: y + 0.1, w: 0.5, h: 0.5, fontSize: 13, bold: true, fontFace: "Trebuchet MS", color: "FFFFFF", align: "center", valign: "middle", margin: 0 });
    s.addText(p.label, { x: x + 0.72, y: y + 0.12, w: 3.6, h: 0.3, fontSize: 13, bold: true, fontFace: "Trebuchet MS", color: GOLD, margin: 0 });
    s.addText(p.text,  { x: x + 0.72, y: y + 0.44, w: 3.6, h: 1.0, fontSize: 11.5, fontFace: "Calibri", color: WHITE, margin: 0 });
  });
}

// ═══════════════════════════════════════════════════════════
// SLIDE 6 — Tree (Fractal)
// ═══════════════════════════════════════════════════════════
{
  const s = pres.addSlide();
  s.background = { color: BG };
  s.addShape(pres.shapes.RECTANGLE, { x: 0, y: 0, w: 10, h: 0.08, fill: { color: ACCENT }, line: { color: ACCENT, width: 0 } });

  accentBar(s, 0.32, 0.62);
  sectionTitle(s, "Tree — Fractal Recursive Model");
  badge(s, "ENVIRONMENT", 0.62, 1.05);

  // Full-width main card
  card(s, 0.35, 1.42, 9.3, 1.75);
  s.addText("Recursive Algorithm", { x: 0.55, y: 1.5, w: 3.5, h: 0.3, fontSize: 13, bold: true, fontFace: "Trebuchet MS", color: GOLD, margin: 0 });
  s.addText(
    "drawFractalTreeRec() is called recursively to depth 4. Each branch is a scaled cube (thickness × length × thickness) with bark texture. " +
    "Each node spawns 3 children: Left (+25°), Right (−25°), Straight (~5°), each with additional random Z-axis rotation for organic variety. " +
    "Branch scale shrinks by 0.65× per level. At depth = 0, a green sphere replaces the branch as a leaf cluster.",
    { x: 0.55, y: 1.82, w: 9.0, h: 1.2, fontSize: 12, fontFace: "Calibri", color: WHITE, margin: 0 }
  );

  // Three detail cards below
  const dtl = [
    { color: "5D4037", title: "Trunk & Branches", lines: ["Cube primitive per branch", "Bark texture applied", "Color: brown → green with depth", "Scale: 0.65× per recursion level"] },
    { color: "388E3C", title: "Leaf Clusters",    lines: ["Sphere at terminal nodes", "Scale = 6 × final thickness", "Color: green (0.0, 0.6, 0.0)", "Applied with leaf texture"] },
    { color: "0277BD", title: "Parameters",       lines: ["Max depth: 4 levels", "Initial length: 2.5 units", "Initial thickness: 0.8 units", "3 children per branch node"] },
  ];
  dtl.forEach((d, i) => {
    const x = 0.35 + i * 3.1;
    card(s, x, 3.35, 2.95, 2.0);
    s.addShape(pres.shapes.RECTANGLE, { x, y: 3.35, w: 2.95, h: 0.33, fill: { color: d.color }, line: { color: d.color, width: 0 } });
    s.addText(d.title, { x: x + 0.1, y: 3.37, w: 2.7, h: 0.28, fontSize: 11.5, bold: true, fontFace: "Trebuchet MS", color: "FFFFFF", margin: 0 });
    const runs = d.lines.map((l, li) => ({ text: l, options: { bullet: { indent: 10 }, color: WHITE, fontSize: 10.5, fontFace: "Calibri", breakLine: li < d.lines.length - 1 } }));
    s.addText(runs, { x: x + 0.1, y: 3.72, w: 2.75, h: 1.55, valign: "top", margin: [3, 4, 3, 4], paraSpaceAfter: 3 });
  });
}

// ═══════════════════════════════════════════════════════════
// SLIDE 7 — Cloud
// ═══════════════════════════════════════════════════════════
{
  const s = pres.addSlide();
  s.background = { color: BG };
  s.addShape(pres.shapes.RECTANGLE, { x: 0, y: 0, w: 10, h: 0.08, fill: { color: ACCENT }, line: { color: ACCENT, width: 0 } });

  accentBar(s, 0.32, 0.62);
  sectionTitle(s, "Cloud");
  badge(s, "ENVIRONMENT", 0.62, 1.05);

  card(s, 0.35, 1.42, 9.3, 3.9);

  const rows = [
    { label: "Structure",    text: "8 overlapping spheres with varying sizes and offsets form one puff. Spheres are arranged in a cluster pattern — center, left, right, top variations — all at off-white color (0.95, 0.97, 1.0)." },
    { label: "Lighting",     text: "High ambient (0.8 × color), low diffuse (0.3 × color), zero specular — produces a uniformly bright, self-lit look without harsh shading typical for clouds." },
    { label: "Placement",    text: "5 cloud instances scattered across the sky at varied x/z positions; each drawn at scale 2.5× for convincing sky coverage." },
    { label: "Sample Puffs", text: "Center (0,0,0) at scale(3.0, 2.5, 2.8) — Right (+1.5,−0.2,+0.5) at scale(2.2, 1.8, 2.0) — Left (−1.8,−0.1,−0.3) at scale(2.4, 1.9, 2.2) + 5 more." },
  ];

  rows.forEach((r, i) => {
    const y = 1.58 + i * 0.85;
    if (i > 0) s.addShape(pres.shapes.LINE, { x: 0.55, y, w: 9.0, h: 0, line: { color: "1A3550", width: 0.8 } });
    s.addText(r.label, { x: 0.55, y: y + 0.08, w: 1.6, h: 0.28, fontSize: 12, bold: true, fontFace: "Trebuchet MS", color: GOLD, margin: 0 });
    s.addText(r.text,  { x: 2.25, y: y + 0.08, w: 7.2, h: 0.62, fontSize: 12, fontFace: "Calibri", color: WHITE, margin: 0 });
  });
}

// ═══════════════════════════════════════════════════════════
// SLIDE 8 — Mountain
// ═══════════════════════════════════════════════════════════
{
  const s = pres.addSlide();
  s.background = { color: BG };
  s.addShape(pres.shapes.RECTANGLE, { x: 0, y: 0, w: 10, h: 0.08, fill: { color: ACCENT }, line: { color: ACCENT, width: 0 } });

  accentBar(s, 0.32, 0.62);
  sectionTitle(s, "Mountain — Background Scenery");
  badge(s, "ENVIRONMENT", 0.62, 1.05);

  // Left description card
  card(s, 0.35, 1.42, 5.5, 3.95);
  s.addText("Composition", { x: 0.55, y: 1.55, w: 4.5, h: 0.3, fontSize: 13, bold: true, fontFace: "Trebuchet MS", color: GOLD, margin: 0 });

  const compLines = [
    "7 scaled & rotated cubes per mountain instance",
    "Massive base slab + central peak + snowcap",
    "Side peaks and outcrops at various rotation angles",
    "Rock texture applied to all except white snowcap",
    "Color shades: 75% → 90% → 115% of base rock color",
    "5 mountain instances placed around scene perimeter",
  ];
  const runs1 = compLines.map((l, i) => ({ text: l, options: { bullet: { indent: 12 }, color: WHITE, fontSize: 12, fontFace: "Calibri", breakLine: i < compLines.length - 1 } }));
  s.addText(runs1, { x: 0.55, y: 1.9, w: 5.0, h: 2.8, valign: "top", margin: [4, 6, 4, 6], paraSpaceAfter: 5 });

  // Right card — atmospheric perspective
  card(s, 6.0, 1.42, 3.7, 3.95);
  s.addText("Atmospheric\nPerspective", { x: 6.15, y: 1.55, w: 3.4, h: 0.65, fontSize: 13, bold: true, fontFace: "Trebuchet MS", color: GOLD, margin: 0 });

  s.addText(
    "Each cube's color is blended 40% toward sky blue (0.4, 0.5, 0.65):",
    { x: 6.15, y: 2.24, w: 3.4, h: 0.5, fontSize: 11, fontFace: "Calibri", color: WHITE, margin: 0 }
  );

  const formula = "finalColor =\n0.6 × rockColor\n+ 0.4 × skyBlue";
  s.addShape(pres.shapes.RECTANGLE, { x: 6.25, y: 2.8, w: 3.35, h: 1.1, fill: { color: "071020" }, line: { color: ACCENT, width: 0.8 } });
  s.addText(formula, { x: 6.25, y: 2.8, w: 3.35, h: 1.1, fontSize: 12.5, fontFace: "Consolas", color: ACCENT, align: "center", valign: "middle", margin: 0 });

  s.addText("Simulates depth haze — farther mountains appear more blue/grey.", {
    x: 6.15, y: 4.0, w: 3.4, h: 0.9, fontSize: 11, fontFace: "Calibri", color: MUTED, margin: 0, italic: true
  });
}

// ═══════════════════════════════════════════════════════════
// SLIDE 9 — Building
// ═══════════════════════════════════════════════════════════
{
  const s = pres.addSlide();
  s.background = { color: BG };
  s.addShape(pres.shapes.RECTANGLE, { x: 0, y: 0, w: 10, h: 0.08, fill: { color: ACCENT }, line: { color: ACCENT, width: 0 } });

  accentBar(s, 0.32, 0.62);
  sectionTitle(s, "Building — Tavern / Structure");
  badge(s, "STRUCTURE", 0.62, 1.05);

  // Two column cards
  const cols = [
    {
      title: "Exterior Shell", color: "4E5D6C",
      items: [
        "Left, right & back walls from textured cubes",
        "Front wall with door opening cut-out",
        "Wooden door rotated −45°, with door handle",
        "Overhanging roof cube slightly wider than base",
        "Ground floor: 2 windows — Upper floor: 3 windows",
        "Blue glass color (0.4, 0.6, 0.8) for window panes",
      ]
    },
    {
      title: "Interior Details", color: "1B4332",
      items: [
        "Bar counter: scaled cube (0.45W × 0.9 × 0.8)",
        "3 bar stools: legs + seat cubes",
        "Wooden table: center post + flat top",
        "3 chairs: seat, back-rest, legs per chair",
        "Shelves with decorative bottle cubes",
        "Hanging ceiling lantern (0.2 × 0.2 × 0.2)",
      ]
    }
  ];

  cols.forEach((col, i) => {
    const x = 0.35 + i * 4.78;
    card(s, x, 1.42, 4.55, 3.92);
    s.addShape(pres.shapes.RECTANGLE, { x, y: 1.42, w: 4.55, h: 0.35, fill: { color: col.color }, line: { color: col.color, width: 0 } });
    s.addText(col.title, { x: x + 0.12, y: 1.44, w: 4.3, h: 0.3, fontSize: 13, bold: true, fontFace: "Trebuchet MS", color: "FFFFFF", margin: 0 });
    const runs = col.items.map((l, li) => ({ text: l, options: { bullet: { indent: 12 }, color: WHITE, fontSize: 11.5, fontFace: "Calibri", breakLine: li < col.items.length - 1 } }));
    s.addText(runs, { x: x + 0.12, y: 1.83, w: 4.3, h: 3.35, valign: "top", margin: [4, 6, 4, 6], paraSpaceAfter: 5 });
  });
}

// ═══════════════════════════════════════════════════════════
// SLIDE 10 — Projectiles
// ═══════════════════════════════════════════════════════════
{
  const s = pres.addSlide();
  s.background = { color: BG };
  s.addShape(pres.shapes.RECTANGLE, { x: 0, y: 0, w: 10, h: 0.08, fill: { color: ACCENT }, line: { color: ACCENT, width: 0 } });

  accentBar(s, 0.32, 0.62);
  sectionTitle(s, "Projectiles — Bullet & Arrow");
  badge(s, "WEAPON", 0.62, 1.05);

  // Bullet card
  card(s, 0.35, 1.42, 4.5, 3.95);
  s.addShape(pres.shapes.RECTANGLE, { x: 0.35, y: 1.42, w: 4.5, h: 0.35, fill: { color: "8B6914" }, line: { color: "8B6914", width: 0 } });
  s.addText("Bullet", { x: 0.5, y: 1.44, w: 4.2, h: 0.3, fontSize: 14, bold: true, fontFace: "Trebuchet MS", color: "FFFFFF", margin: 0 });

  const bulletParts = [
    { label: "Casing:", text: "scale(0.025, 0.025, 0.12) — Brass color (0.78, 0.57, 0.11)" },
    { label: "Tip:", text: "scale(0.020, 0.020, 0.06) at z+0.08 — Copper (0.65, 0.40, 0.15)" },
    { label: "Primer:", text: "scale(0.018, 0.018, 0.005) at z−0.065 — Dark gray" },
    { label: "Rotation:", text: "Quaternion (glm::toMat4) aligns all 3 cubes to velocity vector" },
    { label: "Shininess:", text: "96 (casing) / 80 (tip) / 32 (primer) for realistic metal sheen" },
  ];
  const bRuns = [];
  bulletParts.forEach((p, i) => {
    bRuns.push({ text: p.label + " ", options: { bold: true, color: GOLD, fontSize: 11.5, fontFace: "Calibri" } });
    bRuns.push({ text: p.text, options: { color: WHITE, fontSize: 11.5, fontFace: "Calibri", breakLine: i < bulletParts.length - 1 } });
  });
  s.addText(bRuns, { x: 0.5, y: 1.85, w: 4.1, h: 3.35, valign: "top", margin: [4, 6, 4, 6], paraSpaceAfter: 7 });

  // Arrow card
  card(s, 5.1, 1.42, 4.55, 3.95);
  s.addShape(pres.shapes.RECTANGLE, { x: 5.1, y: 1.42, w: 4.55, h: 0.35, fill: { color: "6B3A2A" }, line: { color: "6B3A2A", width: 0 } });
  s.addText("Arrow", { x: 5.25, y: 1.44, w: 4.2, h: 0.3, fontSize: 14, bold: true, fontFace: "Trebuchet MS", color: "FFFFFF", margin: 0 });

  const arrowParts = [
    { label: "Shaft:", text: "Single cube scale(0.032, 0.032, 0.95) — warm wood color" },
    { label: "Arrowhead:", text: "2 perpendicular thin cubes (cross-blade) at z = +0.54" },
    { label: "Collar:", text: "Gold ring cube scale(0.048, 0.048, 0.03) at z = +0.42" },
    { label: "Fletching:", text: "3 feather cubes at z = −0.40 with 0° / 90° / 45° orientations" },
    { label: "Nock:", text: "Dark brown cap scale(0.04, 0.04, 0.025) at tail z = −0.48" },
  ];
  const aRuns = [];
  arrowParts.forEach((p, i) => {
    aRuns.push({ text: p.label + " ", options: { bold: true, color: GOLD, fontSize: 11.5, fontFace: "Calibri" } });
    aRuns.push({ text: p.text, options: { color: WHITE, fontSize: 11.5, fontFace: "Calibri", breakLine: i < arrowParts.length - 1 } });
  });
  s.addText(aRuns, { x: 5.25, y: 1.85, w: 4.2, h: 3.35, valign: "top", margin: [4, 6, 4, 6], paraSpaceAfter: 7 });
}

// ═══════════════════════════════════════════════════════════
// SLIDE 11 — First-Person Weapons
// ═══════════════════════════════════════════════════════════
{
  const s = pres.addSlide();
  s.background = { color: BG };
  s.addShape(pres.shapes.RECTANGLE, { x: 0, y: 0, w: 10, h: 0.08, fill: { color: ACCENT }, line: { color: ACCENT, width: 0 } });

  accentBar(s, 0.32, 0.62);
  sectionTitle(s, "First-Person Weapons (FPS View)");
  badge(s, "FPS OVERLAY", 0.62, 1.05);

  const weapons = [
    {
      title: "Gun", color: "37474F",
      lines: [
        "~12 cube components assembled per frame in screen space",
        "Barrel: scale(0.022, 0.022, 0.28) — gunmetal grey",
        "Barrel bore (dark hole) at tip z = 0.325",
        "Receiver/slide with 4 decorative serration cuts",
        "Grip rotated 12° on X-axis for natural hold angle",
        "Trigger guard (bar + post), trigger (rotated 20°), front sight",
      ]
    },
    {
      title: "Arrow (Bezier Shaft)", color: "5D4037",
      lines: [
        "14-segment shaft using cubic Bezier control points",
        "Control pts: Cp0(−0.35), Cp1(−0.12), Cp2(+0.12), Cp3(+0.35)",
        "Thickness tapers: 0.013 × (1 − 0.25 × halfDist) per segment",
        "Cross-blade arrowhead: scale(0.045, 0.009, 0.15) × 2",
        "Gold collar ring + 3 fletching feathers at −0.40 offset",
        "Rendered as FPS overlay — no depth conflict with scene",
      ]
    }
  ];

  weapons.forEach((w, i) => {
    const x = 0.35 + i * 4.78;
    card(s, x, 1.42, 4.55, 3.92);
    s.addShape(pres.shapes.RECTANGLE, { x, y: 1.42, w: 4.55, h: 0.35, fill: { color: w.color }, line: { color: w.color, width: 0 } });
    s.addText(w.title, { x: x + 0.12, y: 1.44, w: 4.3, h: 0.3, fontSize: 13, bold: true, fontFace: "Trebuchet MS", color: "FFFFFF", margin: 0 });
    const runs = w.lines.map((l, li) => ({ text: l, options: { bullet: { indent: 12 }, color: WHITE, fontSize: 11.5, fontFace: "Calibri", breakLine: li < w.lines.length - 1 } }));
    s.addText(runs, { x: x + 0.12, y: 1.83, w: 4.3, h: 3.35, valign: "top", margin: [4, 6, 4, 6], paraSpaceAfter: 5 });
  });
}

// ═══════════════════════════════════════════════════════════
// SLIDE 12 — Water, Beach & Ship
// ═══════════════════════════════════════════════════════════
{
  const s = pres.addSlide();
  s.background = { color: BG };
  s.addShape(pres.shapes.RECTANGLE, { x: 0, y: 0, w: 10, h: 0.08, fill: { color: ACCENT }, line: { color: ACCENT, width: 0 } });

  accentBar(s, 0.32, 0.62);
  sectionTitle(s, "Water, Beach & Ship");
  badge(s, "ENVIRONMENT", 0.62, 1.05);

  const sections = [
    { title: "Ocean Water", color: "023E8A", w: 2.9,
      lines: ["4 very large flat cubes per side", "Example: scale(500, 0.5, 160)", "Dark blue-grey (0.2, 0.3, 0.5)", "Static — no wave simulation", "Surrounds island on all sides"] },
    { title: "Beach / Shoreline", color: "C9A84C", w: 2.9,
      lines: ["Thin flat cubes along each island edge", "Tan color (0.8, 0.7, 0.5) — sandy look", "scale(150, 0.15, 8) per beach strip", "Slightly above water level", "Smooth transition to grass"] },
    { title: "Ship", color: "1B5E20", w: 3.1,
      lines: ["Hull: outer + inner + keel cubes", "Red waterline stripe + plank cubes", "Mast 5 units tall with boom & rigging", "Main sail: textured (sail.jpg)", "Cabin with windows; anchor chain detail"] },
  ];

  let xOff = 0.35;
  sections.forEach((sec) => {
    card(s, xOff, 1.42, sec.w, 3.95);
    s.addShape(pres.shapes.RECTANGLE, { x: xOff, y: 1.42, w: sec.w, h: 0.35, fill: { color: sec.color }, line: { color: sec.color, width: 0 } });
    s.addText(sec.title, { x: xOff + 0.1, y: 1.44, w: sec.w - 0.2, h: 0.3, fontSize: 12.5, bold: true, fontFace: "Trebuchet MS", color: "FFFFFF", margin: 0 });
    const runs = sec.lines.map((l, li) => ({ text: l, options: { bullet: { indent: 12 }, color: WHITE, fontSize: 11, fontFace: "Calibri", breakLine: li < sec.lines.length - 1 } }));
    s.addText(runs, { x: xOff + 0.1, y: 1.83, w: sec.w - 0.15, h: 3.35, valign: "top", margin: [4, 6, 4, 6], paraSpaceAfter: 5 });
    xOff += sec.w + 0.15;
  });
}

// ═══════════════════════════════════════════════════════════
// SLIDE 13 — Rain Particle System
// ═══════════════════════════════════════════════════════════
{
  const s = pres.addSlide();
  s.background = { color: BG };
  s.addShape(pres.shapes.RECTANGLE, { x: 0, y: 0, w: 10, h: 0.08, fill: { color: ACCENT }, line: { color: ACCENT, width: 0 } });

  accentBar(s, 0.32, 0.62);
  sectionTitle(s, "Rain — Particle System");
  badge(s, "WEATHER / FX", 0.62, 1.05);

  card(s, 0.35, 1.42, 9.3, 3.9);

  const rows = [
    { label: "Particle Shape",  text: "Each raindrop is a tiny cube scaled (0.02 × 0.23 × 0.02) — very tall and thin to mimic a falling streak." },
    { label: "Tilt & Color",    text: "Rotated 14° on the X-axis to simulate falling at a wind angle; pale blue color (0.6, 0.7, 0.9)." },
    { label: "Particle Count",  text: "Up to 500 active Raindrop structs; each stores position, fall speed, and an active flag." },
    { label: "Spawn & Update",  text: "Particles spawn at random (x, z) above the scene; fall downward each frame by speed × deltaTime, reset at ground level." },
    { label: "Culling",         text: "Only raindrops within 100 units of the camera are drawn — avoids wasting GPU on off-screen particles." },
  ];

  rows.forEach((r, i) => {
    const y = 1.56 + i * 0.7;
    if (i > 0) s.addShape(pres.shapes.LINE, { x: 0.55, y, w: 9.0, h: 0, line: { color: "1A3550", width: 0.8 } });
    s.addText(r.label, { x: 0.55, y: y + 0.06, w: 1.85, h: 0.28, fontSize: 12, bold: true, fontFace: "Trebuchet MS", color: GOLD, margin: 0 });
    s.addText(r.text,  { x: 2.48, y: y + 0.06, w: 7.0, h: 0.52, fontSize: 12, fontFace: "Calibri", color: WHITE, margin: 0 });
  });
}

// ═══════════════════════════════════════════════════════════
// SLIDE 14 — Sky, Sun & Moon
// ═══════════════════════════════════════════════════════════
{
  const s = pres.addSlide();
  s.background = { color: BG };
  s.addShape(pres.shapes.RECTANGLE, { x: 0, y: 0, w: 10, h: 0.08, fill: { color: ACCENT }, line: { color: ACCENT, width: 0 } });

  accentBar(s, 0.32, 0.62);
  sectionTitle(s, "Sky, Sun & Moon");
  badge(s, "ENVIRONMENT", 0.62, 1.05);

  // Three state cards
  const states = [
    { title: "Day Sky",      color: "1565C0", bg: "0A2744",
      lines: ["No sky mesh — uses glClearColor", "Color: (0.53, 0.81, 0.92) — sky blue", "glClear clears color + depth buffer", "Sun: sphere radius 5, yellow (1.0, 1.0, 0.4)"] },
    { title: "Night Sky",    color: "0D1B2A", bg: "070D14",
      lines: ["Background: (0.05, 0.05, 0.1) — dark navy", "Moon: same sphere, cool white (0.9, 0.9, 1.0)", "Ambient light reduced for night effect", "Point lights illuminate scene at night"] },
    { title: "Thunder / Rain", color: "37474F", bg: "1A1F26",
      lines: ["Flash color: (0.40, 0.40, 0.45) — stormy grey", "Switches between states on thunder event", "Rain particles enabled when isRaining = true", "isThunder flag triggers brief bright flash"] },
  ];

  states.forEach((st, i) => {
    const x = 0.35 + i * 3.12;
    s.addShape(pres.shapes.RECTANGLE, { x, y: 1.42, w: 2.97, h: 3.95, fill: { color: st.bg }, line: { color: st.color, width: 1.5 }, shadow: mkShadow() });
    s.addShape(pres.shapes.RECTANGLE, { x, y: 1.42, w: 2.97, h: 0.38, fill: { color: st.color }, line: { color: st.color, width: 0 } });
    s.addText(st.title, { x: x + 0.1, y: 1.44, w: 2.75, h: 0.33, fontSize: 12.5, bold: true, fontFace: "Trebuchet MS", color: "FFFFFF", margin: 0 });
    const runs = st.lines.map((l, li) => ({ text: l, options: { bullet: { indent: 10 }, color: WHITE, fontSize: 11, fontFace: "Calibri", breakLine: li < st.lines.length - 1 } }));
    s.addText(runs, { x: x + 0.1, y: 1.87, w: 2.77, h: 3.35, valign: "top", margin: [4, 6, 4, 6], paraSpaceAfter: 5 });
  });

  // Note about sun/moon
  s.addText("Sun & Moon share a single sphere object — only color and position change based on time-of-day flag.", {
    x: 0.45, y: 5.25, w: 9.1, h: 0.28,
    fontSize: 10.5, fontFace: "Calibri", color: MUTED, align: "center", italic: true
  });
}

// ═══════════════════════════════════════════════════════════
// SLIDE 15 — Rendering Pipeline Summary
// ═══════════════════════════════════════════════════════════
{
  const s = pres.addSlide();
  s.background = { color: BG };
  s.addShape(pres.shapes.RECTANGLE, { x: 0, y: 0, w: 10, h: 0.08, fill: { color: ACCENT }, line: { color: ACCENT, width: 0 } });

  accentBar(s, 0.32, 0.62);
  sectionTitle(s, "Rendering Pipeline — Draw Order");

  const steps = [
    { n: "1", label: "Sky",           text: "glClearColor sets background; glClear clears color + depth buffers",         color: "1565C0" },
    { n: "2", label: "Sun / Moon",    text: "Sphere drawn with simple ourShader (no lighting calc)",                      color: "F9A825" },
    { n: "3", label: "Clouds → Ground → Road", text: "Environment layer: 5 clouds, grass island, asphalt road",          color: "2E7D32" },
    { n: "4", label: "Trees / Mountains / Buildings / Fence", text: "Detailed scenery: fractal trees, multi-cube mountains, taverns, fence posts", color: "5D4037" },
    { n: "5", label: "Tower → Balloons → Projectiles", text: "Gameplay objects drawn in order; balloons may flash red on game-over", color: "C62828" },
    { n: "6", label: "Water → Beach → Ship → Dock", text: "Water surroundings, sandy borders, sailing vessel, pier structure",   color: "01579B" },
    { n: "7", label: "Rain Particles", text: "500 raindrop cubes drawn if isRaining is active (conditionally enabled)",   color: "4A6FA5" },
    { n: "8", label: "FPS Weapon Overlay", text: "Gun or Arrow drawn in screen space on top of 3D scene",                color: "4E5D6C" },
    { n: "9", label: "HUD — Score Display", text: "7-segment digit display rendered orthographically in pixel space",     color: GOLD },
  ];

  // Grid: 3 columns × 3 rows
  steps.forEach((step, i) => {
    const col = i % 3;
    const row = Math.floor(i / 3);
    const x = 0.35 + col * 3.17;
    const y = 1.32 + row * 1.38;

    card(s, x, y, 3.0, 1.25);
    // Step number
    s.addShape(pres.shapes.RECTANGLE, { x: x + 0.1, y: y + 0.1, w: 0.45, h: 0.45, fill: { color: step.color }, line: { color: step.color, width: 0 } });
    s.addText(step.n, { x: x + 0.1, y: y + 0.1, w: 0.45, h: 0.45, fontSize: 13, bold: true, fontFace: "Trebuchet MS", color: "FFFFFF", align: "center", valign: "middle", margin: 0 });
    s.addText(step.label, { x: x + 0.65, y: y + 0.1, w: 2.25, h: 0.35, fontSize: 10.5, bold: true, fontFace: "Trebuchet MS", color: GOLD, margin: 0 });
    s.addText(step.text,  { x: x + 0.1,  y: y + 0.52, w: 2.82, h: 0.65, fontSize: 9.5, fontFace: "Calibri", color: WHITE, margin: 0 });
  });

  // Footer note
  s.addText("All objects share a single cubeVAO (36 indices / 24 vertices).  Sphere and Cone have their own dedicated VAOs.", {
    x: 0.45, y: 5.27, w: 9.1, h: 0.25,
    fontSize: 10, fontFace: "Calibri", color: MUTED, align: "center", italic: true
  });
}

// ── Save ──────────────────────────────────────────────────
pres.writeFile({ fileName: "H:\\projets\\Ballonh\\TowerDefenseBalloonShooter\\Environment_Object_Modelling.pptx" })
  .then(() => console.log("Saved: Environment_Object_Modelling.pptx"))
  .catch(err => { console.error("Error:", err); process.exit(1); });
