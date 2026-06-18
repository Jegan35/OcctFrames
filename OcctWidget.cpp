// ==========================================
// 1. ALL QT HEADERS MUST BE INCLUDED FIRST
// ==========================================
#include <QMouseEvent>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <Graphic3d_MaterialAspect.hxx> // For metallic rendering
#include <QTimer>
#include <Graphic3d_HorizontalTextAlignment.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <gp_Trsf.hxx>
#include <Font_FontAspect.hxx>
#include <kdl/frames.hpp>
#include "RightPanel.h"

// ==========================================
// 2. LOCAL HEADER INCLUDED SECOND
// ==========================================
#include "OcctWidget.h"

// ==========================================
// 3. OPENCASCADE & X11 HEADERS INCLUDED LAST
// ==========================================
#if defined(Q_OS_WIN)
#include <WNT_Window.hxx>
#elif defined(Q_OS_MAC)
#include <Cocoa_Window.hxx>
#else
#include <Xw_Window.hxx>
#include <X11/Xlib.h>
#endif

#include <Aspect_DisplayConnection.hxx>
#include <AIS_Shape.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <gp_Pnt.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <Geom_Axis2Placement.hxx>
#include <AIS_Trihedron.hxx>
#include <StlAPI_Reader.hxx>
#include <RWStl.hxx>
#include <Poly_Triangulation.hxx>
#include <AIS_Triangulation.hxx>
#include <TopLoc_Location.hxx>
#include <AIS_ViewCube.hxx>
#include <Graphic3d_TransformPers.hxx>
#include <AIS_TextLabel.hxx>
#include <TCollection_ExtendedString.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <XCAFApp_Application.hxx>
#include <TDocStd_Document.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <TDF_LabelSequence.hxx>
#include <TopoDS_Compound.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <cmath>
#include <Poly.hxx>
#include <Prs3d_Drawer.hxx>
#include <Prs3d_ShadingAspect.hxx>
#include <Graphic3d_MaterialAspect.hxx>
#include <BRepTools.hxx>
#include <Geom_Surface.hxx>
#include <gp_Quaternion.hxx>
#include <gp_EulerSequence.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopoDS_Wire.hxx>

static gp_Pnt g_partCenter(0, 0, 0);
static bool g_hasPartCenter = false;
static double g_currentRx = 0.0, g_currentRy = 0.0, g_currentRz = 0.0;

OcctWidget::OcctWidget(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_DontCreateNativeAncestors); // ✅ MUST BE HERE

    setMouseTracking(true);
    myCSVPath = "/home/texsonics/Videos/extracted_paths.txt";
}
OcctWidget::~OcctWidget() {}


// =================================================================
// CREATES 3D ARROWS (REDUCED THICKNESS & SLEEK DESIGN)
// =================================================================
Handle(AIS_ColoredShape) OcctWidget::createThickTriad(double scale)
{
    TopoDS_Compound triad;
    BRep_Builder builder;
    builder.MakeCompound(triad);

    // 🚀 THE FIX: தடிமனை (Thickness) மற்றும் நீளத்தைக் கணிசமாகக் குறைத்துள்ளோம்!
    double cylR = 2.5 * scale;   // உருளையின் தடிமன் (முன்பு 7.0 இருந்தது)
    double cylL = 60.0 * scale;  // நீளம் (முன்பு 80.0 இருந்தது)
    double coneR = 7.0 * scale;  // அம்புக்குறியின் தடிமன் (முன்பு 16.0 இருந்தது)
    double coneL = 18.0 * scale; // அம்புக்குறியின் நீளம் (முன்பு 25.0 இருந்தது)

    // X Axis
    TopoDS_Shape xCyl = BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(0,0,0), gp_Dir(1,0,0)), cylR, cylL).Shape();
    TopoDS_Shape xCone = BRepPrimAPI_MakeCone(gp_Ax2(gp_Pnt(cylL,0,0), gp_Dir(1,0,0)), coneR, 0, coneL).Shape();

    // Y Axis
    TopoDS_Shape yCyl = BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(0,0,0), gp_Dir(0,1,0)), cylR, cylL).Shape();
    TopoDS_Shape yCone = BRepPrimAPI_MakeCone(gp_Ax2(gp_Pnt(0,cylL,0), gp_Dir(0,1,0)), coneR, 0, coneL).Shape();

    // Z Axis
    TopoDS_Shape zCyl = BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(0,0,0), gp_Dir(0,0,1)), cylR, cylL).Shape();
    TopoDS_Shape zCone = BRepPrimAPI_MakeCone(gp_Ax2(gp_Pnt(0,0,cylL), gp_Dir(0,0,1)), coneR, 0, coneL).Shape();

    builder.Add(triad, xCyl); builder.Add(triad, xCone);
    builder.Add(triad, yCyl); builder.Add(triad, yCone);
    builder.Add(triad, zCyl); builder.Add(triad, zCone);

    // மெஷ் ஆப்டிமைசேஷன்
    BRepMesh_IncrementalMesh(triad, 5.0);

    Handle(AIS_ColoredShape) coloredTriad = new AIS_ColoredShape(triad);

    coloredTriad->Attributes()->SetFaceBoundaryDraw(Standard_False);
    coloredTriad->Attributes()->SetIsoOnTriangulation(Standard_False);

    Quantity_Color darkRed(0.7, 0.0, 0.0, Quantity_TOC_RGB);
    Quantity_Color darkGreen(0.0, 0.6, 0.0, Quantity_TOC_RGB);
    Quantity_Color darkBlue(0.0, 0.0, 0.7, Quantity_TOC_RGB);

    coloredTriad->SetCustomColor(xCyl, darkRed);
    coloredTriad->SetCustomColor(xCone, darkRed);

    coloredTriad->SetCustomColor(yCyl, darkGreen);
    coloredTriad->SetCustomColor(yCone, darkGreen);

    coloredTriad->SetCustomColor(zCyl, darkBlue);
    coloredTriad->SetCustomColor(zCone, darkBlue);

    return coloredTriad;
}

void OcctWidget::initOCCT()
{
    if (!myView.IsNull()) return;

    Handle(Aspect_DisplayConnection) displayConnection = new Aspect_DisplayConnection();
    myGraphicDriver = new OpenGl_GraphicDriver(displayConnection);

    myViewer = new V3d_Viewer(myGraphicDriver);
    myViewer->SetDefaultLights();
    myViewer->SetLightOn();

    myView = myViewer->CreateView();

#if defined(Q_OS_WIN)
    Handle(WNT_Window) wind = new WNT_Window((Aspect_Handle)winId());
#elif defined(Q_OS_MAC)
    Handle(Cocoa_Window) wind = new Cocoa_Window((NSView *)winId());
#else
    Handle(Xw_Window) wind = new Xw_Window(displayConnection, static_cast<Window>(winId()));
#endif

    myView->SetWindow(wind);
    if (!wind->IsMapped()) {
        wind->Map();
    }

    // ==========================================
    // ✅ FIX 1: INDEPENDENT BACKGROUND COLORS
    // ==========================================
    if (myRole == MainRole) {
        // Left Panel gets the bright, professional gradient
        myView->SetBgGradientColors(Quantity_NOC_WHITE, Quantity_NOC_GRAY90, Aspect_GFM_VER);
    } else {
        // Right Panel gets the pure Black Screen Box!
        myView->SetBackgroundColor(Quantity_NOC_BLACK);
    }

    myContext = new AIS_InteractiveContext(myViewer);

    // Hardware Anti-Aliasing for smooth lines (1080p Optimized)
    myView->ChangeRenderingParams().IsAntialiasingEnabled = Standard_True;

    // 🚀 THE FIX: 1080p-க்கு 8x MSAA போதுமானது. பர்ஃபாமென்ஸ் (Performance) அருமையாக இருக்கும்.
    myView->ChangeRenderingParams().NbMsaaSamples = 8;

    // 🚀 NEW: கோடுகளை (Lines) மிகவும் மென்மையாகக் காட்ட
    myView->ChangeRenderingParams().LineFeather = 1.0;

    // ==========================================
    // ✅ FIX 2: ISOLATE THE GRID AND VIEWCUBE
    // ==========================================
    if (myRole == MainRole) {
        Handle(AIS_ViewCube) viewCube = new AIS_ViewCube();

        viewCube->SetDrawAxes(Standard_True);
        viewCube->SetSize(55);
        viewCube->SetFontHeight(12);
        viewCube->SetAxesLabels("X", "Y", "Z");

        Handle(Graphic3d_TransformPers) trsfPers = new Graphic3d_TransformPers(
            Graphic3d_TMF_TriedronPers,
            Aspect_TOTP_RIGHT_UPPER,
            Graphic3d_Vec2i(85, 85)
            );
        viewCube->SetTransformPersistence(trsfPers);
        myContext->Display(viewCube, Standard_False);

        // ✅ ONLY draw the giant 4000x4000 room grid if this is the Main Left Panel!
        // This stops the Right Panel from doing a massive "FitAll" zoom jerk!
        drawRoomGrid();
    }
}

void OcctWidget::loadStepFile(const std::string& filePath)
{
    if (myView.IsNull()) initOCCT();
    clearSelections();

    QString qFilePath = QString::fromStdString(filePath);

    // =======================================================
    // 🚀 DXF READER (With Try-Catch Crash Shield)
    // =======================================================
    if (qFilePath.endsWith(".dxf", Qt::CaseInsensitive)) {
        QFile file(qFilePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            emit statusUpdate("❌ Error: Cannot open DXF file.");
            return;
        }

        QTextStream in(&file);
        TopoDS_Compound comp;
        BRep_Builder builder;
        builder.MakeCompound(comp);

        double x1 = 0, y1 = 0, z1 = 0, x2 = 0, y2 = 0, z2 = 0;
        bool inLine = false;
        int currentCode = -1;
        int edgesAdded = 0;

        auto saveEdgeSafely = [&](double _x1, double _y1, double _z1, double _x2, double _y2, double _z2) {
            gp_Pnt p1(_x1, _y1, _z1);
            gp_Pnt p2(_x2, _y2, _z2);
            if (p1.Distance(p2) < 0.01) return false;
            try {
                BRepBuilderAPI_MakeEdge edgeMaker(p1, p2);
                if (edgeMaker.IsDone()) {
                    builder.Add(comp, edgeMaker.Shape());
                    return true;
                }
            } catch (...) {
                return false;
            }
            return false;
        };

        // =======================================================
        // 🚀 DXF READER (Strict Pair-Reading & Ghost Line Fix)
        // =======================================================
        bool isCodeLine = true; // Tracks if we are reading a Code or a Value

        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();

            if (isCodeLine) {
                currentCode = line.toInt();
                isCodeLine = false; // Next line will be the value
            } else {
                // We are reading a VALUE line
                if (currentCode == 0) {
                    // Code 0 ALWAYS means a new entity is starting
                    if (line == "LINE") {
                        if (inLine) {
                            if (saveEdgeSafely(x1, y1, z1, x2, y2, z2)) edgesAdded++;
                        }
                        inLine = true;
                        x1 = y1 = z1 = x2 = y2 = z2 = 0.0;
                    } else {
                        // Some other entity started (ARC, CIRCLE, EOF, etc.)
                        if (inLine) {
                            if (saveEdgeSafely(x1, y1, z1, x2, y2, z2)) edgesAdded++;
                            inLine = false; // Turn off to prevent bleed-over!
                        }
                    }
                } else if (inLine) {
                    // We are actively inside a LINE, read its coordinates
                    double val = line.toDouble();
                    if (currentCode == 10) x1 = val;
                    else if (currentCode == 20) y1 = val;
                    else if (currentCode == 30) z1 = val;
                    else if (currentCode == 11) x2 = val;
                    else if (currentCode == 21) y2 = val;
                    else if (currentCode == 31) z2 = val;
                }

                isCodeLine = true; // Next line will be a code
            }
        }

        // Catch the very last line if the file ends abruptly
        if (inLine) {
            if (saveEdgeSafely(x1, y1, z1, x2, y2, z2)) edgesAdded++;
        }
        if (edgesAdded > 0) {
            try {
                // 🚀 THE FIX: DXF மையத்தைக் கண்டுபிடித்து, அதையே நிரந்தர Origin ஆக்குகிறோம்
                Bnd_Box boundingBox;
                BRepBndLib::Add(comp, boundingBox);
                Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
                boundingBox.Get(xMin, yMin, zMin, xMax, yMax, zMax);

                gp_Pnt center((xMin + xMax) / 2.0, (yMin + yMax) / 2.0, (zMin + zMax) / 2.0);
                gp_Trsf centerTrsf;
                centerTrsf.SetTranslation(gp_Vec(-center.X(), -center.Y(), -center.Z()));


                TopoDS_Shape centeredShape = BRepBuilderAPI_Transform(comp, centerTrsf).Shape();
                myLoadedPart = new AIS_Shape(centeredShape);

                // =========================================================
                // 🚀 THE FIX: EXTREME HIGH CONTRAST (CHARCOAL BLACK)
                // This is a very dark slate/black. It will perfectly contrast
                // against your light background AND the bright red path.
                // =========================================================
                Quantity_Color dxfDarkColor(0.10, 0.10, 0.12, Quantity_TOC_RGB);

                myContext->SetColor(myLoadedPart, dxfDarkColor, Standard_False);
                myContext->SetWidth(myLoadedPart, 1.5, Standard_False);

                myContext->Display(myLoadedPart, Standard_True);

                if (myRole == OcctWidget::SideRole) { myView->FitAll(); }
                myView->Redraw();
                setSelectionMode(myCurrentSelectionMode);
                emit statusUpdate(QString("✅ DXF Centered & Loaded (%1 Lines).").arg(edgesAdded));
            } catch (...) {
                emit statusUpdate("❌ Error: DXF contains severely corrupted geometry.");
            }
        }
        return;
    }

    // =======================================================
    // 🚀 STEP READER
    // =======================================================
    Handle(TDocStd_Document) aDoc;
    Handle(XCAFApp_Application) anApp = XCAFApp_Application::GetApplication();
    anApp->NewDocument("MDTV-XCAF", aDoc);

    STEPCAFControl_Reader reader;
    reader.SetColorMode(Standard_True);
    reader.SetNameMode(Standard_True);

    IFSelect_ReturnStatus stat = reader.ReadFile(filePath.c_str());

    if (stat == IFSelect_RetDone) {
        reader.Transfer(aDoc);
        Handle(XCAFDoc_ShapeTool) aShapeTool = XCAFDoc_DocumentTool::ShapeTool(aDoc->Main());
        TDF_LabelSequence labels;
        aShapeTool->GetFreeShapes(labels);

        if (labels.Length() > 0) {
            TDF_Label aLabel = labels.Value(1);

            // 🚀 THE FIX: STEP மையத்தைக் கண்டுபிடித்து, அதையே நிரந்தர Origin ஆக்குகிறோம்
            TopoDS_Shape baseShape = XCAFDoc_ShapeTool::GetShape(aLabel);
            Bnd_Box boundingBox;
            BRepBndLib::Add(baseShape, boundingBox);
            Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
            boundingBox.Get(xMin, yMin, zMin, xMax, yMax, zMax);

            gp_Pnt center((xMin + xMax) / 2.0, (yMin + yMax) / 2.0, (zMin + zMax) / 2.0);
            gp_Trsf centerTrsf;
            centerTrsf.SetTranslation(gp_Vec(-center.X(), -center.Y(), -center.Z()));

            // பார்ட்டை மையத்திற்கு நகர்த்துகிறோம்
            TopoDS_Shape centeredShape = BRepBuilderAPI_Transform(baseShape, centerTrsf).Shape();

            myLoadedPart = new AIS_Shape(centeredShape);
            myContext->SetColor(myLoadedPart, Quantity_NOC_GRAY75, Standard_False);
            myContext->SetMaterial(myLoadedPart, Graphic3d_NOM_ALUMINIUM, Standard_False);

            myContext->SetDisplayMode(myLoadedPart, 1, Standard_False);
            myContext->Display(myLoadedPart, Standard_True);

            if (myRole == OcctWidget::SideRole) { myView->FitAll(); }
            myView->Redraw();
            setSelectionMode(myCurrentSelectionMode);
            emit statusUpdate("✅ Workpiece Centered & Loaded.");
        }
    }
}




void OcctWidget::resetOrigin()
{
    if (myOriginMarker.IsNull()) {
        emit statusUpdate("⚠️ Warning: No model loaded to reset.");
        return;
    }

    if (QMessageBox::question(this, "Reset Origin", "Are you sure you want to reset the origin back to the default Center of Mass?") == QMessageBox::Yes) {
        myCustomOrigin = myDefaultOrigin;

        gp_Ax2 defaultCoords(myCustomOrigin, gp_Dir(0, 0, 1), gp_Dir(1, 0, 0));
        Handle(Geom_Axis2Placement) placement = new Geom_Axis2Placement(defaultCoords);
        myOriginMarker->SetComponent(placement);
        myContext->Redisplay(myOriginMarker, Standard_True);

        emit statusUpdate("🔄 Origin Reset to Default Center of Mass. Note: CSV data will use this new origin on your next Path action.");
    }
}

void OcctWidget::setSelectionMode(int mode)
{
    if(myContext.IsNull()) return;

    myCurrentSelectionMode = mode;

    // 1. Deactivate everything (This accidentally puts the ViewCube to sleep!)
    myContext->Deactivate();

    // 2. Turn on your new Selection rules for the robot parts
    switch(mode) {
    case 1: myContext->Activate(AIS_Shape::SelectionMode(TopAbs_FACE)); break;
    case 2: myContext->Activate(AIS_Shape::SelectionMode(TopAbs_EDGE)); break;
    case 3: myContext->Activate(AIS_Shape::SelectionMode(TopAbs_WIRE)); break;
    default: myContext->Activate(0);
    }

    // ==========================================
    // ✅ THE FIX: WAKE THE VIEW CUBE BACK UP!
    // ==========================================
    // We search the screen for the ViewCube and force its interactivity back on.
    AIS_ListOfInteractive displayedObjects;
    myContext->DisplayedObjects(displayedObjects);
    for (const Handle(AIS_InteractiveObject)& obj : displayedObjects) {
        if (obj->DynamicType() == STANDARD_TYPE(AIS_ViewCube)) {
            myContext->Activate(obj, 0); // 0 is the default click mode
        }
    }
}
void OcctWidget::enableOriginSelectionMode()
{
    myIsSettingOriginMode = true;
    emit statusUpdate("🎯 Origin Mode ACTIVE: Click any edge, face, or wire on the 3D model to snap the origin to it.");
}

// ====================================================================
// NEW CORE ARCHITECTURE: History, Undo, Redo & CSV Generation
// ====================================================================

void OcctWidget::clearSelections()
{
    // Remove all red lines from the screen
    for (const auto& step : myPathHistory) myContext->Remove(step.visualRedPath, Standard_False);
    for (const auto& step : myRedoStack) myContext->Remove(step.visualRedPath, Standard_False);

    // Clear the Start Marker
    if (!myStartPointMarker.IsNull()) {
        myContext->Remove(myStartPointMarker, Standard_False);
        myStartPointMarker.Nullify();
    }
    if (!myStartLabel.IsNull()) {
        myContext->Remove(myStartLabel, Standard_False);
        myStartLabel.Nullify();
    }

    myPathHistory.clear();
    myRedoStack.clear();
    myContext->UpdateCurrentViewer();

    // 🚀 THE FIX: Do NOT call regenerateCSV() here, otherwise it destroys the saved file.
    // Instead, send an empty string to the Right Panel to clear the UI text box.
    emit coordinatesExtracted("");

    emit statusUpdate("❌ All marks and selections cleared.");
}
// ==========================================================
// 🚀 NEW: DRAW "START" POINT MARKER ON THE PART
// ==========================================================
void OcctWidget::drawStartMarker(const gp_Pnt& pt)
{
    if (myContext.IsNull()) return;

    // 1. Create a bright Green Sphere (Radius 4.0mm)
    TopoDS_Shape sphere = BRepPrimAPI_MakeSphere(pt, 4.0).Shape();
    myStartPointMarker = new AIS_Shape(sphere);
    myContext->SetColor(myStartPointMarker, Quantity_NOC_GREEN, Standard_False);
    myContext->SetMaterial(myStartPointMarker, Graphic3d_NOM_PLASTIC, Standard_False);
    myContext->SetDisplayMode(myStartPointMarker, 1, Standard_False);
    myContext->Display(myStartPointMarker, Standard_False);

    // 2. Create the "START" Text Label floating slightly above the point
    myStartLabel = new AIS_TextLabel();
    myStartLabel->SetText(TCollection_ExtendedString("START"));
    myStartLabel->SetPosition(gp_Pnt(pt.X(), pt.Y(), pt.Z() + 15.0)); // Lift it 15mm up
    myStartLabel->SetHeight(22);
    myStartLabel->SetColor(Quantity_NOC_GREEN);
    myStartLabel->SetFontAspect(Font_FA_Bold);
    myStartLabel->SetZoomable(Standard_True);
    myContext->Display(myStartLabel, Standard_False);
}
void OcctWidget::undoSelection()
{
    if (myPathHistory.empty()) {
        emit statusUpdate("⚠️ Nothing to undo.");
        return;
    }

    // Move from Active History to Redo Stack
    PathData lastAction = myPathHistory.back();
    myPathHistory.pop_back();
    myContext->Remove(lastAction.visualRedPath, Standard_True); // Hide the red line
    myRedoStack.push_back(lastAction);

    regenerateCSV();
    emit statusUpdate(QString("↩️ Undo successful. Current Paths in CSV: %1").arg(myPathHistory.size()));
}

void OcctWidget::redoSelection()
{
    if (myRedoStack.empty()) {
        emit statusUpdate("⚠️ Nothing to redo.");
        return;
    }

    // Move from Redo Stack back to Active History
    PathData nextAction = myRedoStack.back();
    myRedoStack.pop_back();
    myContext->Display(nextAction.visualRedPath, Standard_True); // Show the red line again
    myPathHistory.push_back(nextAction);

    regenerateCSV();
    emit statusUpdate(QString("↪️ Redo successful. Current Paths in CSV: %1").arg(myPathHistory.size()));
}

// Update the function signature to take the parameter
void OcctWidget::processCurrentSelection(double resolution)
{
    if (myContext.IsNull() || !myContext->HasSelectedShape()) return;

    if (!(QApplication::keyboardModifiers() & Qt::ShiftModifier)) {
        clearSelections();
        m_isFirstPointFound = false;
    }

    myContext->InitSelected();
    int addedCount = 0;

    QString txtData;
    QTextStream stringOut(&txtData);
    // 🚀 THE FIX: Removed the "X,Y,Z\n" header!

    while (myContext->MoreSelected()) {
        TopoDS_Shape shape = myContext->SelectedShape();

        Handle(AIS_Shape) plottedPath = new AIS_Shape(shape);
        myContext->SetColor(plottedPath, Quantity_NOC_RED, Standard_False);

        if (shape.ShapeType() == TopAbs_FACE) {
            myContext->SetDisplayMode(plottedPath, 1, Standard_False);
        } else {
            myContext->SetWidth(plottedPath, 3.0, Standard_False);
        }

        myContext->Display(plottedPath, Standard_True);
        myPathHistory.push_back({shape, plottedPath, resolution});
        addedCount++;

        switch (shape.ShapeType()) {
        case TopAbs_FACE: processFace(TopoDS::Face(shape), stringOut, resolution); break;
        case TopAbs_WIRE: processWire(TopoDS::Wire(shape), stringOut, resolution); break;
        case TopAbs_EDGE: processEdge(TopoDS::Edge(shape), stringOut, resolution); break;
        default: break;
        }

        myContext->NextSelected();
    }
    myContext->ClearSelected(Standard_True);
    myRedoStack.clear();

    emit coordinatesExtracted(txtData);
    regenerateCSV();
    emit statusUpdate(QString("✅ Extracted %1 new path(s). Total Paths: %2").arg(addedCount).arg(myPathHistory.size()));
    emit selectionChanged(false);
}

void OcctWidget::regenerateCSV()
{
    // 🚀 THE FIX: If there is no active path in memory, DO NOT wipe the physical file!
    // This keeps your last generated points perfectly safe when the app closes.
    if (myPathHistory.empty()) {
        return;
    }

    QFile file(myCSVPath);
    // QIODevice::Truncate will safely overwrite the old points ONLY if we have new ones to save
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return;
    }

    QTextStream out(&file);

    for (const auto& step : myPathHistory) {
        switch (step.shape.ShapeType()) {
        case TopAbs_FACE: processFace(TopoDS::Face(step.shape), out, step.resolution); break;
        case TopAbs_WIRE: processWire(TopoDS::Wire(step.shape), out, step.resolution); break;
        case TopAbs_EDGE: processEdge(TopoDS::Edge(step.shape), out, step.resolution); break;
        default: break;
        }
    }

    file.close();
}





static gp_Dir g_faceNormal(0, 0, 1);
static bool g_hasFaceNormal = false;


void OcctWidget::processFace(const TopoDS_Face& face, QTextStream& out, double resolution)
{
    Standard_Real umin, umax, vmin, vmax;
    BRepTools::UVBounds(face, umin, umax, vmin, vmax);
    Handle(Geom_Surface) surf = BRep_Tool::Surface(face);

    gp_Pnt p; gp_Vec d1u, d1v;
    surf->D1((umin + umax) / 2.0, (vmin + vmax) / 2.0, p, d1u, d1v);

    gp_Vec normVec = d1u.Crossed(d1v);
    if (face.Orientation() == TopAbs_REVERSED) normVec.Reverse();

    g_faceNormal = gp_Dir(normVec);
    g_hasFaceNormal = true;

    TopExp_Explorer wireExplorer(face, TopAbs_WIRE);
    for (; wireExplorer.More(); wireExplorer.Next()) {
        TopoDS_Wire wire = TopoDS::Wire(wireExplorer.Current());

        // 🚀 THE FIX: Removed the Boundary Marker text!
        processWire(wire, out, resolution);
    }
    g_hasFaceNormal = false;
}

void OcctWidget::processWire(const TopoDS_Wire& wire, QTextStream& out, double resolution)
{
    BRepAdaptor_CompCurve compCurve(wire, Standard_True);
    Standard_Real first = compCurve.FirstParameter();
    Standard_Real last = compCurve.LastParameter();
    GCPnts_UniformAbscissa discretizer(compCurve, resolution, first, last);

    if (discretizer.IsDone()) {
        for (int i = 1; i <= discretizer.NbPoints(); ++i) {
            Standard_Real param = discretizer.Parameter(i);
            gp_Pnt pt;
            gp_Vec tangentVec;
            compCurve.D1(param, pt, tangentVec);

            // =========================================================
            // 🚀 RESTORED ORIGINAL MATH: The axes that actually worked!
            // =========================================================
            gp_Dir normal = g_hasFaceNormal ? g_faceNormal : gp_Dir(0, 0, 1);
            gp_Dir x_axis(-normal.X(), -normal.Y(), -normal.Z());
            gp_Dir y_axis(tangentVec);

            if (x_axis.IsParallel(y_axis, 0.01)) {
                y_axis = gp_Dir(0, 0, 1);
                if (x_axis.IsParallel(y_axis, 0.01)) y_axis = gp_Dir(0, 1, 0);
            }

            gp_Dir z_axis = x_axis.Crossed(y_axis);
            y_axis = z_axis.Crossed(x_axis);

            gp_Ax3 defaultOXY(gp_Pnt(0,0,0), gp_Dir(0,0,1), gp_Dir(1,0,0));
            gp_Ax3 toolPos(pt, z_axis, x_axis);

            if (!m_isFirstPointFound) {
                drawStartMarker(toolPos.Location());
                m_isFirstPointFound = true;
            }

            gp_Trsf inverseTranslation;
            inverseTranslation.SetTranslation(gp_Vec(-myCustomOrigin.X(), -myCustomOrigin.Y(), -myCustomOrigin.Z()));
            toolPos.Transform(inverseTranslation);

            gp_Trsf trsf;
            trsf.SetDisplacement(defaultOXY, toolPos);

            // =========================================================
            // 🧠 THE FIX: ONE BRAIN (Route OpenCASCADE through KDL)
            // =========================================================
            // 1. Get the 3x3 Matrix from OpenCASCADE
            gp_Mat m = trsf.VectorialPart();

            // 2. Convert it into a KDL Rotation Matrix
            KDL::Rotation kdlRot(
                m.Value(1,1), m.Value(1,2), m.Value(1,3),
                m.Value(2,1), m.Value(2,2), m.Value(2,3),
                m.Value(3,1), m.Value(3,2), m.Value(3,3)
                );

            // 3. Ask the Brain for the unified degrees
            double rx, ry, rz;
            RobotMath::getUnifiedEulerDegrees(kdlRot, rx, ry, rz);

            // =========================================================
            // 🚀 RESTORED: MILLIMETERS
            // =========================================================
            double x_mm = toolPos.Location().X();
            double y_mm = toolPos.Location().Y();
            double z_mm = toolPos.Location().Z();

            // Note: rx, ry, rz are ALREADY in degrees thanks to RobotMath
            out << x_mm << "," << y_mm << "," << z_mm << ","
                << rx << "," << ry << "," << rz << "\n";
        }
    }
}

void OcctWidget::processEdge(const TopoDS_Edge& edge, QTextStream& out, double resolution)
{
    Standard_Real first, last;
    Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, first, last);
    if (curve.IsNull()) return;

    bool isTrimmedEdge = (!m_customStartEdge.IsNull() && edge.IsSame(m_customStartEdge));

    if (isTrimmedEdge) {
        Standard_Real origFirst = first;
        Standard_Real origLast = last;
        double pctStart = m_trimStartPct / 100.0;
        double pctEnd = m_trimEndPct / 100.0;

        if (pctStart > pctEnd) {
            first = origFirst + (origLast - origFirst) * pctEnd;
            last = origFirst + (origLast - origFirst) * pctStart;
        } else {
            first = origFirst + (origLast - origFirst) * pctStart;
            last = origFirst + (origLast - origFirst) * pctEnd;
        }
    }

    BRepAdaptor_Curve adaptor(edge);
    GCPnts_UniformAbscissa discretizer(adaptor, resolution, first, last);

    if (discretizer.IsDone()) {
        for (int i = 1; i <= discretizer.NbPoints(); ++i) {
            Standard_Real param = discretizer.Parameter(i);
            gp_Pnt pt;
            gp_Vec tangentVec;
            adaptor.D1(param, pt, tangentVec);

            // =========================================================
            // 🚀 RESTORED ORIGINAL MATH
            // =========================================================
            gp_Dir normal = g_hasFaceNormal ? g_faceNormal : gp_Dir(0, 0, 1);
            gp_Dir x_axis(-normal.X(), -normal.Y(), -normal.Z());
            gp_Dir y_axis(tangentVec);

            if (x_axis.IsParallel(y_axis, 0.01)) {
                y_axis = gp_Dir(0, 0, 1);
                if (x_axis.IsParallel(y_axis, 0.01)) y_axis = gp_Dir(0, 1, 0);
            }

            gp_Dir z_axis = x_axis.Crossed(y_axis);
            y_axis = z_axis.Crossed(x_axis);

            gp_Ax3 defaultOXY(gp_Pnt(0,0,0), gp_Dir(0,0,1), gp_Dir(1,0,0));
            gp_Ax3 toolPos(pt, z_axis, x_axis);

            if (!m_isFirstPointFound) {
                if (!isTrimmedEdge) {
                    drawStartMarker(toolPos.Location());
                }
                m_isFirstPointFound = true;
            }

            gp_Trsf inverseTranslation;
            inverseTranslation.SetTranslation(gp_Vec(-myCustomOrigin.X(), -myCustomOrigin.Y(), -myCustomOrigin.Z()));
            toolPos.Transform(inverseTranslation);

            gp_Trsf trsf;
            trsf.SetDisplacement(defaultOXY, toolPos);

            // =========================================================
            // 🧠 THE FIX: ONE BRAIN (Route OpenCASCADE through KDL)
            // =========================================================
            gp_Mat m = trsf.VectorialPart();

            KDL::Rotation kdlRot(
                m.Value(1,1), m.Value(1,2), m.Value(1,3),
                m.Value(2,1), m.Value(2,2), m.Value(2,3),
                m.Value(3,1), m.Value(3,2), m.Value(3,3)
                );

            double rx, ry, rz;
            RobotMath::getUnifiedEulerDegrees(kdlRot, rx, ry, rz);

            // =========================================================
            // 🚀 RESTORED: MILLIMETERS
            // =========================================================
            double x_mm = toolPos.Location().X();
            double y_mm = toolPos.Location().Y();
            double z_mm = toolPos.Location().Z();

            // Note: rx, ry, rz are ALREADY in degrees thanks to RobotMath
            out << x_mm << "," << y_mm << "," << z_mm << ","
                << rx << "," << ry << "," << rz << "\n";
        }
    }
}


void OcctWidget::paintEvent(QPaintEvent *event)
{
    // ✅ MUST BE CALLED: Keeps Qt's internal rendering loop happy
    QWidget::paintEvent(event);

    if (myView.IsNull()) initOCCT();
    myView->Redraw();
}

void OcctWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (!myView.IsNull()) {
        myView->MustBeResized(); // Tell X11 to update the dimensions

        // ✅ NEW: Automatically re-frame the camera when the layout updates!
        // This ensures the grid and text are never cut off after the "MAX" fix.
        myView->FitAll();

        myView->Redraw();
    }
}
void OcctWidget::mousePressEvent(QMouseEvent *event)
{
    myLastMousePos = event->pos();
    int x = event->pos().x() * devicePixelRatio();
    int y = event->pos().y() * devicePixelRatio();

    if (event->button() == Qt::LeftButton) {
        myContext->MoveTo(x, y, myView, Standard_True);

        if (event->modifiers() & Qt::ShiftModifier) {
            myContext->SelectDetected(AIS_SelectionScheme_XOR);
        } else {
            myContext->SelectDetected(AIS_SelectionScheme_Replace);
        }

        myView->Redraw();
        myContext->InitSelected();
        if (myContext->HasSelectedShape()) {

            // ✅ THE SHIELD: If we clicked the ViewCube, STOP here and let it move the camera!
            Handle(AIS_InteractiveObject) selObj = myContext->SelectedInteractive();
            if (!selObj.IsNull() && selObj->DynamicType() == STANDARD_TYPE(AIS_ViewCube)) {
                myContext->ClearSelected(Standard_False);
                return;
            }

            if (myIsSettingOriginMode) {
                // ... (Keep all your existing origin setup logic below here) ...
                // (Keep your existing origin setup logic here...)
                TopoDS_Shape selectedShape = myContext->SelectedShape();
                Bnd_Box boundingBox;
                BRepBndLib::Add(selectedShape, boundingBox);
                Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
                boundingBox.Get(xMin, yMin, zMin, xMax, yMax, zMax);
                gp_Pnt newOriginSnap((xMin + xMax) / 2.0, (yMin + yMax) / 2.0, (zMin + zMax) / 2.0);

                QString msg = QString("Do you want to set the new Robot Origin here?\n\nX: %1\nY: %2\nZ: %3")
                                  .arg(newOriginSnap.X(), 0, 'f', 2).arg(newOriginSnap.Y(), 0, 'f', 2).arg(newOriginSnap.Z(), 0, 'f', 2);

                if (QMessageBox::question(this, "Confirm New Origin", msg) == QMessageBox::Yes) {
                    myCustomOrigin = newOriginSnap;
                    gp_Ax2 newCoords(myCustomOrigin, gp_Dir(0, 0, 1), gp_Dir(1, 0, 0));
                    Handle(Geom_Axis2Placement) placement = new Geom_Axis2Placement(newCoords);
                    myOriginMarker->SetComponent(placement);
                    myContext->Redisplay(myOriginMarker, Standard_True);
                    emit statusUpdate("🎯 New Local Origin Set Successfully!");
                } else {
                    emit statusUpdate("Origin Setup Cancelled.");
                }

                myIsSettingOriginMode = false;
                myContext->ClearSelected(Standard_True);
                return;
            }

            // ==========================================
            // NEW ROLE CHECKING LOGIC
            // ==========================================
            if (myRole == MainRole) {
                // Left Window: Isolate the part and send to Right Window
                TopoDS_Shape selectedShape = myContext->SelectedShape();
                emit partSelectedForIsolation(selectedShape);
                myContext->ClearSelected(Standard_True);
            }
            else if (myRole == SideRole) {
                // ✅ FIX: Don't extract immediately! Just notify the UI that a shape is selected.
                // The user must click "GET POINTS" to actually calculate the math.
                emit selectionChanged(myContext->HasSelectedShape());
            }
        }
    }
    // ==========================================
    // ✅ NEW FIX: Tell the camera to start rotating!
    // ==========================================
    else if (event->button() == Qt::RightButton) {
        myView->StartRotation(x, y);
    }
}










void OcctWidget::mouseMoveEvent(QMouseEvent *event)
{
    int x = event->pos().x() * devicePixelRatio();
    int y = event->pos().y() * devicePixelRatio();

    if (event->buttons() & Qt::RightButton) {
        myView->Rotation(x, y);
    } else if (event->buttons() & Qt::MiddleButton) {
        int lastX = myLastMousePos.x() * devicePixelRatio();
        int lastY = myLastMousePos.y() * devicePixelRatio();
        myView->Pan(x - lastX, lastY - y);
    } else {
        myContext->MoveTo(x, y, myView, Standard_True);
    }
    myLastMousePos = event->pos();
}

void OcctWidget::wheelEvent(QWheelEvent *event)
{
    myView->Zoom(0, 0, event->angleDelta().y() / 10, 0);
}


// ==========================================================
// ISOLATED PART DISPLAY (Right Window)
// ==========================================================
void OcctWidget::displayIsolatedPart(const TopoDS_Shape& shape)
{
    if (myView.IsNull()) initOCCT();

    myContext->RemoveAll(Standard_False);

    Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
    myContext->SetDisplayMode(aisShape, 1, Standard_False);

    myContext->SetColor(aisShape, Quantity_NOC_GRAY75, Standard_False);
    myContext->SetMaterial(aisShape, Graphic3d_NOM_ALUMINIUM, Standard_False);

    myContext->Display(aisShape, Standard_False);

    setSelectionMode(myCurrentSelectionMode);

    myView->FitAll();

    // ADD THIS LINE: Zoom in 40% closer for isolated parts
    myView->SetZoom(1.4);

    myView->Redraw();
}

// ==========================================================
// CALIBRATION OFFSET (Moves the Table/Square)
// ==========================================================
void OcctWidget::offsetWorkpiece(double dx, double dy, double dz)
{
    if (myLoadedPart.IsNull()) {
        emit statusUpdate("⚠️ Load a part first before moving it.");
        return;
    }

    // Create a 3D Translation Vector
    gp_Trsf transform;
    transform.SetTranslation(gp_Vec(dx, dy, dz));
    TopLoc_Location loc(transform);

    // Apply the offset to the part
    myContext->SetLocation(myLoadedPart, loc);
    myContext->UpdateCurrentViewer();

    emit statusUpdate(QString("📏 Part Calibrated to Robot Base -> X:%1, Y:%2, Z:%3").arg(dx).arg(dy).arg(dz));
}



// ==========================================
// 1. TRIGGER THE ASYNC LOADER
// ==========================================
void OcctWidget::loadDefaultRobot()
{
    if (myView.IsNull()) initOCCT();

    // Prevent crashing if the user spam-clicks the Load button
    if (myCurrentLoadIndex != -1) return;

    myCurrentLoadIndex = 0; // Start at link0

    emit statusUpdate("⏳ Loading STL Robot Base (link0)...");

    // Trigger the very first part to load after a tiny 50ms UI pause
    QTimer::singleShot(50, this, &OcctWidget::loadNextRobotLink);
}


void OcctWidget::loadNextRobotLink()
{
    // ==========================================================
    // 🚀 Load all 7 parts (link0 to link6)
    // ==========================================================
    if (myCurrentLoadIndex > 6) {
        myBaseTriad = createThickTriad(1.5);
        myTipTriad = createThickTriad(1.2);

        myContext->SetDisplayMode(myBaseTriad, 1, Standard_False);
        myContext->SetDisplayMode(myTipTriad, 1, Standard_False);

        gp_Trsf globalRot;
        globalRot.SetRotation(gp_Ax1(gp_Pnt(0,0,0), gp_Dir(0,0,1)), -M_PI / 2.0);
        myContext->SetLocation(myBaseTriad, TopLoc_Location(globalRot));

        myContext->Display(myBaseTriad, Standard_False);
        myContext->Display(myTipTriad, Standard_False);

        myContext->Deactivate(myBaseTriad);
        myContext->Deactivate(myTipTriad);

        updateRobotPosture(0, 0, 0, 0, 0, 0);
        setSelectionMode(myCurrentSelectionMode);

        emit statusUpdate("✅ Successfully loaded 7 STL robot links (link0 to link6).");

        myCurrentLoadIndex = -1;
        emit robotLoadComplete();

        // DELAYED CAMERA CENTERING & ZOOM
        QTimer::singleShot(250, this, [this]() {
            if (!myView.IsNull()) {
                myView->MustBeResized();
                myView->FitAll();
                myView->SetZoom(1.3);
                myView->Redraw();
            }
        });

        return;
    }

    // 2. Setup the file path
    QString folderPath = "/home/texsonics/Documents/toolocct/step1/";
    QString fileName = folderPath + QString("link%1.stl").arg(myCurrentLoadIndex);

    if (!QFile::exists(fileName)) {
        qDebug() << "❌ STL FILE NOT FOUND: Missing ->" << fileName;
        myCurrentLoadIndex++;
        QTimer::singleShot(50, this, &OcctWidget::loadNextRobotLink);
        return;
    }

    std::string stdFile = fileName.toStdString();
    Handle(Poly_Triangulation) mesh = RWStl::ReadFile(stdFile.c_str());

    if (!mesh.IsNull()) {

        // 1. Generate 3D lighting faces (fixes the pitch-black issue)
        Poly::ComputeNormals(mesh);

        Handle(AIS_Triangulation) aisShape = new AIS_Triangulation(mesh);

        gp_Trsf zeroTrsf;
        myContext->SetLocation(aisShape, TopLoc_Location(zeroTrsf));

        // 🎨 EXACT QML COLORS CONVERTED TO OPEN CASCADE RGB (0.0 TO 1.0)

        // 🎨 EXACT QML COLORS CONVERTED TO OPEN CASCADE RGB (0.0 TO 1.0)
        Quantity_Color partColor;

        switch (myCurrentLoadIndex) {
        case 0: partColor = Quantity_Color(0.690, 0.690, 0.690, Quantity_TOC_RGB); break; // #b0b0b0
        case 1: partColor = Quantity_Color(0.827, 0.184, 0.184, Quantity_TOC_RGB); break; // #d32f2f
        case 2: partColor = Quantity_Color(0.220, 0.557, 0.235, Quantity_TOC_RGB); break; // #388e3c
        case 3: partColor = Quantity_Color(0.098, 0.463, 0.824, Quantity_TOC_RGB); break; // #1976d2
        case 4: partColor = Quantity_Color(0.984, 0.753, 0.176, Quantity_TOC_RGB); break; // #fbc02d
        case 5: partColor = Quantity_Color(0.482, 0.122, 0.635, Quantity_TOC_RGB); break; // #7b1fa2
        case 6: partColor = Quantity_Color(1.000, 1.000, 1.000, Quantity_TOC_RGB); break; // #ffffff
        default: partColor = Quantity_Color(1.0, 1.0, 1.0, Quantity_TOC_RGB); break;
        }
        // ==========================================================
        // 🚀 Apply Custom Color to Shading Aspect
        // ==========================================================


        //Ashok design
        // 🎨 EXACT COLORS FROM THE UPLOADED IMAGE (RGB: 0.0 to 1.0)
        // Quantity_Color partColor;

        // switch (myCurrentLoadIndex) {
        // case 0: partColor = Quantity_Color(0.85, 0.65, 0.10, Quantity_TOC_RGB); break; // Base: Dark/Mustard Yellow
        // case 1: partColor = Quantity_Color(0.65, 0.65, 0.60, Quantity_TOC_RGB); break; // Link 1: Grey/Khaki
        // case 2: partColor = Quantity_Color(0.65, 0.45, 0.65, Quantity_TOC_RGB); break; // Link 2: Muted Purple
        // case 3: partColor = Quantity_Color(0.45, 0.70, 0.60, Quantity_TOC_RGB); break; // Link 3: Mint/Teal
        // case 4: partColor = Quantity_Color(0.35, 0.50, 0.75, Quantity_TOC_RGB); break; // Link 4: Steel Blue
        // case 5: partColor = Quantity_Color(0.70, 0.70, 0.25, Quantity_TOC_RGB); break; // Link 5: Yellow/Olive
        // case 6: partColor = Quantity_Color(0.80, 0.75, 0.20, Quantity_TOC_RGB); break; // Link 6: Golden Yellow
        // default: partColor = Quantity_Color(1.00, 1.00, 1.00, Quantity_TOC_RGB); break;
        // }

        // ==========================================================
        // 🚀 THE PREMIUM MATERIAL UPGRADE (Shadow & Depth Fix)
        // ==========================================================
        Handle(Prs3d_ShadingAspect) shadingAspect = new Prs3d_ShadingAspect();

        Graphic3d_MaterialAspect premiumMaterial(Graphic3d_NOM_PLASTIC);

        // 1. Base & Diffuse Color (நேரடி வெளிச்சம் படும் இடங்களுக்கு 100% நிறம்)
        premiumMaterial.SetColor(partColor);
        premiumMaterial.SetDiffuseColor(partColor);

        // 2. 🚀 THE MAGIC FIX: நிழல் விழும் பகுதிகளை (Ambient) 30% ஆகக் குறைக்கிறோம்!
        Quantity_Color shadowColor(
            partColor.Red() * 0.1,   // சிவப்பு நிறத்தில் 30%
            partColor.Green() * 0.1, // பச்சை நிறத்தில் 30%
            partColor.Blue() * 0.1,  // நீல நிறத்தில் 30%
            Quantity_TOC_RGB
            );
        premiumMaterial.SetAmbientColor(shadowColor);

        // 3. ✨ Soft Specular Highlight (பிரீமியம் பிளாஸ்டிக் பளபளப்பு)
        premiumMaterial.SetSpecularColor(Quantity_Color(1.0, 1.0, 1.0, Quantity_TOC_RGB));
        premiumMaterial.SetShininess(0.4);

        // ==========================================================

        shadingAspect->SetColor(partColor);
        shadingAspect->SetMaterial(premiumMaterial);

        aisShape->Attributes()->SetShadingAspect(shadingAspect);

        // Fallback for context
        myContext->SetColor(aisShape, partColor, Standard_False);

        myContext->Display(aisShape, Standard_False);
        myRobotLinks.push_back(aisShape);
        myContext->Deactivate(aisShape);
        myView->Redraw();

    } // (if condition-ஐ மூடும் பிராக்கெட்)

    // 3. Increment the tracker to the next part
    myCurrentLoadIndex++;

    // 4. Update the UI text
    emit statusUpdate(QString("⏳ Loading Raw Mesh (link%1.stl)...").arg(myCurrentLoadIndex - 1));

    QTimer::singleShot(50, this, &OcctWidget::loadNextRobotLink);
}







void OcctWidget::updateRobotPosture(double j1, double j2, double j3, double j4, double j5, double j6)
{
    if (myRobotLinks.empty()) return;

    // 1. Base Scale (Assuming STLs are in Meters, scale to MM)
    gp_Trsf baseTrsf;
    baseTrsf.SetScale(gp_Pnt(0,0,0), 1000.0);

    // ========================================================
    // ✅ NEW: GLOBAL WORLD ROTATION
    // This spins the ENTIRE assembled robot 90 degrees around Z
    // to match the facing direction of your main project!
    // ========================================================
    //gp_Trsf globalRot;
    // NOTE: If the robot faces backward, just remove the negative sign -> (M_PI / 2.0)
    //globalRot.SetRotation(gp_Ax1(gp_Pnt(0,0,0), gp_Dir(0,0,1)), -M_PI / 2.0);

    // ========================================================
    // 2. TRUE ABSOLUTE KINEMATIC ORIGINS (In Millimeters)
    // ========================================================
    gp_Pnt orig1(0.0, 0.0, 0.0);        // J1 Pivot (Base)
    gp_Pnt orig2(155.0, 0.0, 470.0);    // J2 Pivot (Shoulder)
    gp_Pnt orig3(155.0, 0.0, 1074.0);   // J3 Pivot (Elbow)
    gp_Pnt orig4(155.0, 0.0, 1274.0);   // J4 Pivot (Forearm Roll)
    gp_Pnt orig5(795.5, 0.0, 1274.0);   // J5 Pivot (Wrist Pitch)
    gp_Pnt orig6(795.5, 0.0, 1274.0);   // J6 Pivot (Wrist Roll - Intersects J5)

    // ========================================================
    // 3. ROTATION AXES (Matching your KDL perfectly)
    // ========================================================
    gp_Dir axis1(0, 0, 1); // J1: RotZ
    gp_Dir axis2(0, 1, 0); // J2: RotY
    gp_Dir axis3(0, 1, 0); // J3: RotY
    gp_Dir axis4(1, 0, 0); // J4: RotX
    gp_Dir axis5(0, 1, 0); // J5: RotY
    gp_Dir axis6(1, 0, 0); // J6: RotX

    // 4. Create pure rotation transformations around those absolute pins
    gp_Trsf R1, R2, R3, R4, R5, R6;
    R1.SetRotation(gp_Ax1(orig1, axis1), j1);
    R2.SetRotation(gp_Ax1(orig2, axis2), j2);
    R3.SetRotation(gp_Ax1(orig3, axis3), j3);
    R4.SetRotation(gp_Ax1(orig4, axis4), j4);
    R5.SetRotation(gp_Ax1(orig5, axis5), j5);
    R6.SetRotation(gp_Ax1(orig6, axis6), j6);

    // ========================================================
    // 5. HIERARCHICAL ACCUMULATION
    // ========================================================
    gp_Trsf accum1 = R1;
    gp_Trsf accum2 = R1 * R2;
    gp_Trsf accum3 = R1 * R2 * R3;
    gp_Trsf accum4 = R1 * R2 * R3 * R4;
    gp_Trsf accum5 = R1 * R2 * R3 * R4 * R5;
    gp_Trsf accum6 = R1 * R2 * R3 * R4 * R5 * R6;

    // ========================================================
    // 🚀 THE FIX: 'globalRot *'
    // ========================================================
    if (myRobotLinks.size() > 0) myContext->SetLocation(myRobotLinks[0], TopLoc_Location(baseTrsf));
    if (myRobotLinks.size() > 1) myContext->SetLocation(myRobotLinks[1], TopLoc_Location(accum1 * baseTrsf));
    if (myRobotLinks.size() > 2) myContext->SetLocation(myRobotLinks[2], TopLoc_Location(accum2 * baseTrsf));
    if (myRobotLinks.size() > 3) myContext->SetLocation(myRobotLinks[3], TopLoc_Location(accum3 * baseTrsf));
    if (myRobotLinks.size() > 4) myContext->SetLocation(myRobotLinks[4], TopLoc_Location(accum4 * baseTrsf));
    if (myRobotLinks.size() > 5) myContext->SetLocation(myRobotLinks[5], TopLoc_Location(accum5 * baseTrsf));

    // If your STL robot has a 7th piece (the tool flange):
    if (myRobotLinks.size() > 6) myContext->SetLocation(myRobotLinks[6], TopLoc_Location(accum6 * baseTrsf));

    // ========================================================
    // ✅ ATTACH THE ARROWS AND TOOL TO THE ROBOT
    // ========================================================
    if (!myTipTriad.IsNull()) {

        // 1. POSITION OF THE BARE FLANGE (J6)
        gp_Trsf moveToFlange;
        moveToFlange.SetTranslation(gp_Vec(795.5 + 100.0, 0.0, 1274.0));


        gp_Trsf flangeTrsf = accum6 * moveToFlange;
        myLastTipTrsf = flangeTrsf;

        // 2. ATTACH THE TOOL STL EXACTLY TO THE FLANGE
        if (!myToolShape.IsNull()) {
            myContext->SetLocation(myToolShape, TopLoc_Location(flangeTrsf));
        }

        // 3. CALCULATE THE TCP (Flange + Tool Offset) FOR THE MARKER
        gp_Trsf toolOffsetTrsf;
        toolOffsetTrsf.SetTranslation(gp_Vec(m_toolOffsetX, m_toolOffsetY, m_toolOffsetZ));
        gp_Trsf tcpTrsf = flangeTrsf * toolOffsetTrsf; // 🚀 Math: Shift by Z=150!

        // 4. MOVE THE MARKER (ARROWS) TO THE REAL TCP TIP!
        myContext->SetLocation(myTipTriad, TopLoc_Location(tcpTrsf));

        // ========================================================
        // ✅ DRAW THE LIVE TRAJECTORY TRAIL FROM THE TCP
        // ========================================================
        gp_Pnt currentTCP = gp_Pnt(0,0,0).Transformed(tcpTrsf);

        if (myTrajectoryPoints.empty() || myTrajectoryPoints.back().Distance(currentTCP) > 1.0) {
            myTrajectoryPoints.push_back(currentTCP);
            if (myTrajectoryPoints.size() > 500) {
                myTrajectoryPoints.erase(myTrajectoryPoints.begin());
            }

            static qint64 lastTrailRedraw = 0;
            qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

            if (myTrajectoryPoints.size() > 1 && (currentTime - lastTrailRedraw > 100)) {
                lastTrailRedraw = currentTime;

                BRepBuilderAPI_MakePolygon polyMaker;
                for (const auto& pt : myTrajectoryPoints) {
                    polyMaker.Add(pt);
                }

                if (polyMaker.IsDone()) {
                    TopoDS_Wire wire = polyMaker.Wire();

                    if (myTrajectoryShape.IsNull()) {
                        myTrajectoryShape = new AIS_Shape(wire);
                        myContext->SetColor(myTrajectoryShape, Quantity_NOC_RED, Standard_False);
                        myContext->SetWidth(myTrajectoryShape, 3.0, Standard_False);
                        myContext->Display(myTrajectoryShape, Standard_False);
                        myContext->Deactivate(myTrajectoryShape);
                    } else {
                        myTrajectoryShape->SetShape(wire);
                        myContext->Redisplay(myTrajectoryShape, Standard_False);
                    }
                }
            }
        }
    }

    // ========================================================
    // 🚀 THE FIX FOR THE HANGING TABS AND STUTTERING
    // ========================================================
    this->update();
}

void OcctWidget::clearMarks()
{
    if (myContext.IsNull()) return;

    // 1. Erase all visual paths from the history stack without updating the screen
    for (size_t i = 0; i < myPathHistory.size(); ++i) {
        if (!myPathHistory[i].visualRedPath.IsNull()) {
            myContext->Remove(myPathHistory[i].visualRedPath, Standard_False);
        }
    }

    // 2. Erase all visual paths from the redo stack without updating the screen
    for (size_t i = 0; i < myRedoStack.size(); ++i) {
        if (!myRedoStack[i].visualRedPath.IsNull()) {
            myContext->Remove(myRedoStack[i].visualRedPath, Standard_False);
        }
    }

    // 3. Clear the actual vectors to free up memory
    myPathHistory.clear();
    myRedoStack.clear();
    myTrajectoryPoints.clear();

    // 4. If you have a single trajectory shape drawn, remove it too
    if (!myTrajectoryShape.IsNull()) {
        myContext->Remove(myTrajectoryShape, Standard_False);
        myTrajectoryShape.Nullify();
    }

    // 5. Update the viewer exactly ONCE after all deletions are complete
    myContext->UpdateCurrentViewer();

    // 6. Regenerate the CSV (which will now be empty) to clear the Right Panel TextEdit
    regenerateCSV();

    emit statusUpdate("✅ All marks and selections cleared instantly.");
}
// ==========================================================
// ✅ THE FIX: Wake up the Native X11 Window when the page flips
// ==========================================================
// ==========================================================
// ✅ THE FIX: Auto-trigger the layout sync (The "MAX" trick)
// ==========================================================
void OcctWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    // Wait 100ms for Qt's layout and the Linux X11 Window Manager
    // to finish settling, then force the 3D canvas to snap to bounds!
    QTimer::singleShot(100, this, [this]() {
        if (!myView.IsNull()) {
            myView->MustBeResized();
            myView->Redraw();
            this->update(); // Force a Qt UI repaint just in case
        }
    });
}

void OcctWidget::clearLoadedPart()
{
    if (!myLoadedPart.IsNull()) {
        myContext->Remove(myLoadedPart, Standard_True);
        myLoadedPart.Nullify();
        emit statusUpdate("🗑️ File cleared from view.");
        myView->Redraw();
    }
}

QString OcctWidget::getOriginText() const {
    return QString("X: %1 | Y: %2 | Z: %3")
    .arg(myCustomOrigin.X(), 0, 'f', 3)
        .arg(myCustomOrigin.Y(), 0, 'f', 3)
        .arg(myCustomOrigin.Z(), 0, 'f', 3);
}

void OcctWidget::drawRoomGrid()
{
    // Defined boundaries
    double minX = -2000.0, maxX = 2000.0;
    double minY = -2000.0, maxY = 2000.0;
    double minZ = 0.0, maxZ = 2000.0;
    double step = 100.0;

    TopoDS_Compound floorComp, backWallComp, leftWallComp;
    BRep_Builder builder;
    builder.MakeCompound(floorComp);
    builder.MakeCompound(backWallComp);
    builder.MakeCompound(leftWallComp);

    // 1. FLOOR (Red Grid)
    for (double x = minX; x <= maxX; x += step)
        builder.Add(floorComp, BRepBuilderAPI_MakeEdge(gp_Pnt(x, minY, minZ), gp_Pnt(x, maxY, minZ)));
    for (double y = minY; y <= maxY; y += step)
        builder.Add(floorComp, BRepBuilderAPI_MakeEdge(gp_Pnt(minX, y, minZ), gp_Pnt(maxX, y, minZ)));

    // ---------------------------------------------------------
    // 2. BACK WALL (Blue Wall - Behind the robot at X = minX)
    // (No changes here)
    // ---------------------------------------------------------
    for (double y = minY; y <= maxY; y += step)
        builder.Add(backWallComp, BRepBuilderAPI_MakeEdge(gp_Pnt(minX, y, minZ), gp_Pnt(minX, y, maxZ)));
    for (double z = minZ; z <= maxZ; z += step)
        builder.Add(backWallComp, BRepBuilderAPI_MakeEdge(gp_Pnt(minX, minY, z), gp_Pnt(minX, maxY, z)));

    // ---------------------------------------------------------
    // 🚀 3. SIDE WALL (Green Wall) - SWAPPED: Moved to Y = minY (Right side of the robot)
    // ---------------------------------------------------------
    for (double x = minX; x <= maxX; x += step)
        builder.Add(leftWallComp, BRepBuilderAPI_MakeEdge(gp_Pnt(x, minY, minZ), gp_Pnt(x, minY, maxZ)));
    for (double z = minZ; z <= maxZ; z += step)
        builder.Add(leftWallComp, BRepBuilderAPI_MakeEdge(gp_Pnt(minX, minY, z), gp_Pnt(maxX, minY, z)));

    Handle(AIS_Shape) aisFloor = new AIS_Shape(floorComp);
    Handle(AIS_Shape) aisBack = new AIS_Shape(backWallComp);
    Handle(AIS_Shape) aisLeft = new AIS_Shape(leftWallComp);

    // ==========================================
    // ✅ COLORS SETTING
    // ==========================================
    Quantity_Color lightRedFloor(Quantity_NOC_INDIANRED1);
    myContext->SetColor(aisFloor, lightRedFloor, Standard_False);
    myContext->SetColor(aisBack, Quantity_NOC_STEELBLUE, Standard_False);
    myContext->SetColor(aisLeft, Quantity_NOC_SEAGREEN, Standard_False);

    // 🚀 THE FIX: 1080p-க்கு 1.5 Thickness வைத்தால் கச்சிதமாகத் தெரியும்.
    myContext->SetWidth(aisFloor, 1.5, Standard_False);
    myContext->SetWidth(aisBack, 1.5, Standard_False);
    myContext->SetWidth(aisLeft, 1.5, Standard_False);

    myContext->Display(aisFloor, Standard_False);
    myContext->Display(aisBack, Standard_False);
    myContext->Display(aisLeft, Standard_False);
    myContext->Deactivate(aisFloor);
    myContext->Deactivate(aisBack);
    myContext->Deactivate(aisLeft);

    int textSize = 60;

    // ==========================================
    // 🚀 RED LABELS (X-Axis / Front-Back) - SWAPPED: Moved to Y = maxY (Left side of the floor)
    // ==========================================
    for (int x = -2000; x <= 2000; x += 100) {
        if (x == 0) continue;
        Handle(AIS_TextLabel) xLabel = new AIS_TextLabel();
        xLabel->SetText(TCollection_ExtendedString(x));
        xLabel->SetPosition(gp_Pnt(x, maxY + 50.0, 0)); // Moved to maxY
        xLabel->SetHeight(textSize);
        xLabel->SetColor(Quantity_NOC_RED);
        xLabel->SetFontAspect(Font_FA_Bold);
        xLabel->SetZoomable(Standard_True);
        myContext->Display(xLabel, Standard_False);
        myContext->Deactivate(xLabel);
    }

    // ==========================================
    // 🚀 GREEN LABELS (Y-Axis / Left-Right)
    // ==========================================
    for (int y = -2000; y <= 2000; y += 100) {
        if (y == 0) continue;
        Handle(AIS_TextLabel) yLabel = new AIS_TextLabel();
        yLabel->SetText(TCollection_ExtendedString(y));
        yLabel->SetPosition(gp_Pnt(maxX + 50.0, y, 0));
        yLabel->SetHeight(textSize);

        // 🚀 THE FIX: GREEN என்பதை DARKGREEN ஆக மாற்றிவிட்டோம்
        yLabel->SetColor(Quantity_NOC_DARKGREEN);

        yLabel->SetFontAspect(Font_FA_Bold);
        yLabel->SetZoomable(Standard_True);
        myContext->Display(yLabel, Standard_False);
        myContext->Deactivate(yLabel);
    }

    // ==========================================
    // 🚀 Z-AXIS LABELS (Blue) - MOVED TO THE OPPOSITE SIDE (maxY)
    // ==========================================
    for (int z = 100; z <= 2000; z += 100) {
        Handle(AIS_TextLabel) zLabel = new AIS_TextLabel();
        zLabel->SetText(TCollection_ExtendedString(z));
        zLabel->SetPosition(gp_Pnt(minX - 50.0, maxY + 50.0, z));

        zLabel->SetHeight(textSize);
        zLabel->SetColor(Quantity_NOC_BLUE1);
        zLabel->SetFontAspect(Font_FA_Bold);
        zLabel->SetZoomable(Standard_True);
        myContext->Display(zLabel, Standard_False);
        myContext->Deactivate(zLabel);
    }

    // ==========================================
    // ✅ PERFECT SMALL POSTER LOGO (On Blue Wall)
    // ==========================================
    Handle(AIS_TextLabel) titleLabel = new AIS_TextLabel();
    titleLabel->SetText(TCollection_ExtendedString("TEXSONICS"));
    titleLabel->SetPosition(gp_Pnt(minX + 1.0, 0.0, 1100));
    titleLabel->SetHeight(60);
    titleLabel->SetColor(Quantity_NOC_BLACK);
    titleLabel->SetZoomable(Standard_True);
    titleLabel->SetOrientation3D(gp_Ax2(gp_Pnt(minX, 0.0, 1200), gp_Dir(1, 0, 0), gp_Dir(0, 1, 0)));
    titleLabel->SetHJustification(Graphic3d_HTA_CENTER);
    myContext->Display(titleLabel, Standard_False);
    myContext->Deactivate(titleLabel);

    Handle(AIS_TextLabel) subLabel = new AIS_TextLabel();
    subLabel->SetText(TCollection_ExtendedString("R O B O T I C S"));
    subLabel->SetPosition(gp_Pnt(minX + 1.0, 0.0, 900));
    subLabel->SetHeight(30);
    subLabel->SetColor(Quantity_NOC_BLACK);
    subLabel->SetZoomable(Standard_True);
    subLabel->SetOrientation3D(gp_Ax2(gp_Pnt(minX, 0.0, 900), gp_Dir(1, 0, 0), gp_Dir(0, 1, 0)));
    subLabel->SetHJustification(Graphic3d_HTA_CENTER);
    myContext->Display(subLabel, Standard_False);
    myContext->Deactivate(subLabel);

    myView->FitAll();
}



// ==========================================================
// ✅ NEW: SET USER FRAME ORIGIN & DRAW 3D MARKER
// ==========================================================


// ==========================================================
// ✅ THE FIX: SET USER FRAME ORIGIN (PRESERVES ROTATION)
// ==========================================================
void OcctWidget::setUserFrameOrigin(double ui_x, double ui_y, double ui_z)
{
    if (myContext.IsNull()) return;

    myCustomOrigin = gp_Pnt(ui_x, ui_y, ui_z);

    // 1. 🚀 RECOVER THE ACTIVE ROTATION (Do not wipe it out!)
    double radX = g_currentRx * (M_PI / 180.0);
    double radY = g_currentRy * (M_PI / 180.0);
    double radZ = g_currentRz * (M_PI / 180.0);

    gp_Trsf rotX, rotY, rotZ, toUserFrame;
    rotX.SetRotation(gp_Ax1(gp_Pnt(0,0,0), gp_Dir(1,0,0)), radX);
    rotY.SetRotation(gp_Ax1(gp_Pnt(0,0,0), gp_Dir(0,1,0)), radY);
    rotZ.SetRotation(gp_Ax1(gp_Pnt(0,0,0), gp_Dir(0,0,1)), radZ);

    toUserFrame.SetTranslation(gp_Vec(ui_x, ui_y, ui_z));

    // Combine into one master transformation matrix
    gp_Trsf finalTrsf = toUserFrame * rotZ * rotY * rotX;

    // 2. Apply it to the 3D Part
    if (!myLoadedPart.IsNull()) {
        myContext->SetLocation(myLoadedPart, TopLoc_Location(finalTrsf));
    }

    // 3. Apply it to the Target Marker Triad (Arrows)
    if (myUserFrameMarker.IsNull()) {
        myUserFrameMarker = createThickTriad(1.3);
        myContext->SetDisplayMode(myUserFrameMarker, 1, Standard_False);
        myContext->Display(myUserFrameMarker, Standard_False);
    }

    myContext->SetLocation(myUserFrameMarker, TopLoc_Location(finalTrsf));

    myContext->UpdateCurrentViewer();
    emit statusUpdate(QString("📍 UserFrame Origin SET at X:%1 Y:%2 Z:%3").arg(ui_x).arg(ui_y).arg(ui_z));
}




// =========================================================================
// TOOL FRAME: CLEAR EXISTING TOOL
// =========================================================================
void OcctWidget::clearToolShape()
{
    if (!myToolShape.IsNull() && !myContext.IsNull()) {
        myContext->Remove(myToolShape, Standard_False);
        myToolShape.Nullify();
    }

    // ✅ RESET OFFSETS TO 0 WHEN TOOL IS DETACHED
    m_toolOffsetX = 0.0;
    m_toolOffsetY = 0.0;
    m_toolOffsetZ = 0.0;

    if (!myContext.IsNull()) myContext->UpdateCurrentViewer();
}

// =========================================================================
// TOOL FRAME: LOAD STL WITH DYNAMIC TOOL PROFILES
// =========================================================================
void OcctWidget::loadToolShapeOnTip(const QString& toolName, double x, double y, double z)
{
    if (myContext.IsNull()) return;
    clearToolShape();

    // 1. Save offsets
    m_toolOffsetX = x;
    m_toolOffsetY = y;
    m_toolOffsetZ = z;

    QString fileName = toolName.trimmed();
    if (!fileName.endsWith(".stl", Qt::CaseInsensitive)) fileName += ".stl";

    QString folderPath = "/home/texsonics/Documents/toolocct/step/";
    QString fullPath = folderPath + fileName;

    TopoDS_Shape toolTopoShape;
    StlAPI_Reader reader;
    if (!reader.Read(toolTopoShape, fullPath.toStdString().c_str())) {
        QMessageBox::critical(this, "Tool Load Error", "Cannot find STL file:\n" + fullPath);
        return;
    }

    // ========================================================================
    // 🛠️ DYNAMIC TOOL PROFILES (எந்த டூல் என்று கண்டுபிடித்து செட்டிங்ஸ் மாற்றுதல்)
    // ========================================================================
    double scaleVal = 1.0;
    double alpha_A = 0.0, beta_B = 0.0, gamma_C = 0.0;
    double offsetCAD_X = 0.0, offsetCAD_Y = 0.0, offsetCAD_Z = 0.0;

    // ----------------------------------------------------
    // 🔧 PROFILE FOR TOOL 1 (The Welding Gun)
    // ----------------------------------------------------
    if (fileName.contains("tool1", Qt::CaseInsensitive)) {
        scaleVal = 1000.0;        // மீட்டரில் இருப்பதால் 1000 மடங்கு பெரிதாக்குகிறோம்
        beta_B = 90.0;            // நேராகத் திருப்புகிறோம்
        offsetCAD_Y = -12.0;      // சென்டர் செய்கிறோம்
        offsetCAD_Z = 68.0;       // மேலே தூக்குகிறோம்
    }
    // ----------------------------------------------------
    // 🔧 PROFILE FOR TOOL 2 (The Blue Cylinder/Bucket)
    // ----------------------------------------------------
    // ----------------------------------------------------
    // 🔧 PROFILE FOR TOOL 2 (The Blue Cylinder/Bucket)
    // ----------------------------------------------------
    else if (fileName.contains("tool2", Qt::CaseInsensitive)) {
        scaleVal = 1000.0;        // 🚀 சைஸ் கரெக்டாக இருப்பதால் இதை மாற்ற வேண்டாம்!

        // 🚀 டூலை நேராக முன்னாடி (Forward) திருப்ப:
        alpha_A = 0.0;
        beta_B = 90.0;            // ⬅️ இதை மட்டும் 90.0 என்று மாற்றுங்கள்! (ஒருவேளை பின்பக்கம் திரும்பினால் -90.0 என மாற்றவும்)
        gamma_C = 0.0;

        // 🚀 சென்டர் பாயிண்ட் கரெக்டாக இருப்பதால் இதையும் மாற்ற வேண்டாம்!
        offsetCAD_X = 0.0;
        offsetCAD_Y = 0.0;
        offsetCAD_Z = 0.0;
    }

    // Transformations
    gp_Trsf rotA, rotB, rotC;
    rotA.SetRotation(gp_Ax1(gp_Pnt(0,0,0), gp_Dir(0,0,1)), alpha_A * (M_PI / 180.0));
    rotB.SetRotation(gp_Ax1(gp_Pnt(0,0,0), gp_Dir(0,1,0)), beta_B  * (M_PI / 180.0));
    rotC.SetRotation(gp_Ax1(gp_Pnt(0,0,0), gp_Dir(1,0,0)), gamma_C * (M_PI / 180.0));
    gp_Trsf finalEulerRotation = rotA * rotB * rotC;

    gp_Trsf cadTranslation;
    cadTranslation.SetTranslation(gp_Vec(offsetCAD_X, offsetCAD_Y, offsetCAD_Z));

    gp_Trsf scaleMatrix;
    scaleMatrix.SetScale(gp_Pnt(0,0,0), scaleVal); // 🚀 டூலுக்கு ஏற்ற Scale!

    gp_Trsf alignmentTransformation = cadTranslation * finalEulerRotation * scaleMatrix;
    toolTopoShape = BRepBuilderAPI_Transform(toolTopoShape, alignmentTransformation).Shape();

    myToolShape = new AIS_Shape(toolTopoShape);
    // 🎨 அடர்த்தியான அசல் ஆரஞ்சு (Deep Industrial Orange)
    Quantity_Color toolColor(1.0, 0.35, 0.0, Quantity_TOC_RGB);

    // எந்த சிக்கலும் இல்லாமல் நேரடியாக நிறத்தையும் பிளாஸ்டிக் லுக்கையும் செட் செய்கிறோம்
    myToolShape->SetColor(toolColor);
    myToolShape->SetMaterial(Graphic3d_NOM_PLASTIC);
    // Attach to Flange
    myContext->SetLocation(myToolShape, TopLoc_Location(myLastTipTrsf));
    myContext->SetDisplayMode(myToolShape, 1, Standard_False);
    myContext->Display(myToolShape, Standard_False);

    // Set Marker to TCP
    if (!myTipTriad.IsNull()) {
        gp_Trsf tcpOffset;
        tcpOffset.SetTranslation(gp_Vec(x, y, z));
        myContext->SetLocation(myTipTriad, TopLoc_Location(myLastTipTrsf * tcpOffset));
    }

    myContext->UpdateCurrentViewer();
}
// ==========================================================
// 🚀 NEW: DRAW RED TARGET SPHERE FOR TCP CALIBRATION
// ==========================================================
void OcctWidget::drawTargetMarker(double x, double y, double z)
{
    if (myContext.IsNull()) return;

    // பழைய புள்ளி இருந்தால் அழித்துவிடு
    if (!myTargetMarker.IsNull()) {
        myContext->Remove(myTargetMarker, Standard_False);
        myTargetMarker.Nullify();
    }

    // 8mm அளவில் ஒரு சிவப்புக் கோளத்தை (Sphere) வரைகிறோம்
    TopoDS_Shape sphere = BRepPrimAPI_MakeSphere(gp_Pnt(x, y, z), 8.0).Shape();
    myTargetMarker = new AIS_Shape(sphere);
    myContext->SetColor(myTargetMarker, Quantity_NOC_RED, Standard_False);
    myContext->SetMaterial(myTargetMarker, Graphic3d_NOM_PLASTIC, Standard_False);
    myContext->SetDisplayMode(myTargetMarker, 1, Standard_False);

    myContext->Display(myTargetMarker, Standard_True);
    myContext->UpdateCurrentViewer();
}
// ==========================================================
// 🚀 NEW: REMOVE RED TARGET SPHERE
// ==========================================================
void OcctWidget::clearTargetMarker()
{
    if (!myContext.IsNull() && !myTargetMarker.IsNull()) {
        myContext->Remove(myTargetMarker, Standard_True);
        myTargetMarker.Nullify();
        myContext->UpdateCurrentViewer();
    }
}




void OcctWidget::transformLoadedPart(double dx, double dy, double dz, double rx, double ry, double rz)
{
    if (myLoadedPart.IsNull()) return;

    // Save current values
    g_currentRx = rx; g_currentRy = ry; g_currentRz = rz;
    myCustomOrigin = gp_Pnt(dx, dy, dz);

    double radX = rx * (M_PI / 180.0);
    double radY = ry * (M_PI / 180.0);
    double radZ = rz * (M_PI / 180.0);

    gp_Trsf rotX, rotY, rotZ, toUserFrame;

    // 🚀 THE FIX: The part is ALREADY centered at 0,0,0 by loadStepFile!
    // We just rotate it in place, then move it to the User Frame XYZ.
    rotX.SetRotation(gp_Ax1(gp_Pnt(0,0,0), gp_Dir(1,0,0)), radX);
    rotY.SetRotation(gp_Ax1(gp_Pnt(0,0,0), gp_Dir(0,1,0)), radY);
    rotZ.SetRotation(gp_Ax1(gp_Pnt(0,0,0), gp_Dir(0,0,1)), radZ);

    toUserFrame.SetTranslation(gp_Vec(dx, dy, dz));

    // Combine into final transformation
    gp_Trsf finalTrsf = toUserFrame * rotZ * rotY * rotX;

    // Apply to the loaded part
    myContext->SetLocation(myLoadedPart, TopLoc_Location(finalTrsf));

    // Apply to the User Frame Marker (Arrows)
    if (!myUserFrameMarker.IsNull()) {
        myContext->SetLocation(myUserFrameMarker, TopLoc_Location(finalTrsf));
    }

    myContext->UpdateCurrentViewer();
}

// ====================================================================
// 🚀 NEW: ONE-CLICK FULL SHAPE EXTRACTION (SORTED & CONTINUOUS)
// ====================================================================
void OcctWidget::processAllEdges(double resolution)
{
    if (myContext.IsNull() || myLoadedPart.IsNull()) {
        emit statusUpdate("⚠️ Load a part first!");
        return;
    }

    clearSelections();
    m_isFirstPointFound = false;
    int addedCount = 0;

    QString txtData;
    QTextStream stringOut(&txtData);

    Handle(AIS_Shape) aisShape = Handle(AIS_Shape)::DownCast(myLoadedPart);
    if (aisShape.IsNull()) {
        emit statusUpdate("⚠️ Error: The loaded part is not a valid CAD shape.");
        return;
    }

    TopoDS_Shape rawShape = aisShape->Shape();
    gp_Trsf currentTrsf = myContext->Location(aisShape).Transformation();
    TopoDS_Shape transformedShape = BRepBuilderAPI_Transform(rawShape, currentTrsf).Shape();

    // ========================================================================
    // 🚀 THE FIX: STITCH LOOSE EDGES INTO A CONTINUOUS, SORTED LOOP!
    // ========================================================================
    TopTools_ListOfShape edgeList;
    TopExp_Explorer edgeExplorer(transformedShape, TopAbs_EDGE);

    // 1. Gather all the random loose lines
    for (; edgeExplorer.More(); edgeExplorer.Next()) {
        edgeList.Append(TopoDS::Edge(edgeExplorer.Current()));
    }

    // 2. Put them in the Wire Maker. It automatically sorts them end-to-end!
    BRepBuilderAPI_MakeWire wireMaker;
    wireMaker.Add(edgeList);

    if (wireMaker.IsDone()) {
        // 3. Extract the perfect continuous loop
        TopoDS_Wire sortedWire = wireMaker.Wire();

        // Turn the continuous loop RED
        Handle(AIS_Shape) plottedPath = new AIS_Shape(sortedWire);
        myContext->SetColor(plottedPath, Quantity_NOC_RED, Standard_False);
        myContext->SetWidth(plottedPath, 3.0, Standard_False);
        myContext->Display(plottedPath, Standard_True);

        myPathHistory.push_back({sortedWire, plottedPath, resolution});
        addedCount++;

        // 4. Send the SORTED loop to the point generator!
        processWire(sortedWire, stringOut, resolution);
    } else {
        emit statusUpdate("⚠️ Could not stitch lines. Make sure the DXF shape is a closed loop!");
        return;
    }
    // ========================================================================

    myRedoStack.clear();
    emit coordinatesExtracted(txtData);
    regenerateCSV();

    emit statusUpdate("✅ FULL SHAPE EXTRACTED as a smooth, continuous path!");
}


// ==========================================================
// 🚀 CALCULATE CUSTOM START POINT & MOVE LABEL
// ==========================================================
void OcctWidget::calculateCustomStartPoint(double percentage)
{
    if (myContext.IsNull() || !myContext->HasSelectedShape()) return;

    TopoDS_Shape shape = myContext->SelectedShape();
    if (shape.ShapeType() != TopAbs_EDGE) return;
    m_customStartEdge = TopoDS::Edge(shape);

    m_trimStartPct = percentage; // Save the math!

    Standard_Real first, last;
    Handle(Geom_Curve) curve = BRep_Tool::Curve(m_customStartEdge, first, last);
    Standard_Real targetParam = first + (last - first) * (percentage / 100.0);

    gp_Pnt rawPt;
    curve->D0(targetParam, rawPt);

    // Draw Sphere
    if (!m_customStartMarker.IsNull()) myContext->Remove(m_customStartMarker, Standard_False);
    TopoDS_Shape sphere = BRepPrimAPI_MakeSphere(rawPt, 4.0).Shape();
    m_customStartMarker = new AIS_Shape(sphere);
    myContext->SetColor(m_customStartMarker, Quantity_NOC_GREEN, Standard_False);
    myContext->Display(m_customStartMarker, Standard_False);

    // Draw "START" Label right above the Sphere!
    if (!m_customStartLabel.IsNull()) myContext->Remove(m_customStartLabel, Standard_False);
    m_customStartLabel = new AIS_TextLabel();
    m_customStartLabel->SetText(TCollection_ExtendedString("START"));
    m_customStartLabel->SetPosition(gp_Pnt(rawPt.X(), rawPt.Y(), rawPt.Z() + 15.0));
    m_customStartLabel->SetHeight(22);
    m_customStartLabel->SetColor(Quantity_NOC_GREEN);
    m_customStartLabel->SetFontAspect(Font_FA_Bold);
    myContext->Display(m_customStartLabel, Standard_True);

    // 🚀 THE FIX: Convert to Robot coords and update the UI box!
    gp_Trsf inverseTranslation;
    inverseTranslation.SetTranslation(gp_Vec(-myCustomOrigin.X(), -myCustomOrigin.Y(), -myCustomOrigin.Z()));
    gp_Pnt robotPt = rawPt.Transformed(inverseTranslation);

    QString xyzText = QString("X: %1 | Y: %2 | Z: %3")
                          .arg(robotPt.X(), 0, 'f', 2)
                          .arg(robotPt.Y(), 0, 'f', 2)
                          .arg(robotPt.Z(), 0, 'f', 2);

    emit customStartPointCalculated(xyzText);
    emit statusUpdate("✅ Start Point Set.");
}


// ==========================================================
// 🚀 CALCULATE CUSTOM END POINT & MOVE LABEL
// ==========================================================
void OcctWidget::calculateCustomEndPoint(double percentage)
{
    if (myContext.IsNull() || !myContext->HasSelectedShape()) return;

    TopoDS_Shape shape = myContext->SelectedShape();
    if (shape.ShapeType() != TopAbs_EDGE) return;
    m_customEndEdge = TopoDS::Edge(shape);

    m_trimEndPct = percentage; // Save the math!

    Standard_Real first, last;
    Handle(Geom_Curve) curve = BRep_Tool::Curve(m_customEndEdge, first, last);
    Standard_Real targetParam = first + (last - first) * (percentage / 100.0);

    gp_Pnt rawPt;
    curve->D0(targetParam, rawPt);

    // Draw Sphere
    if (!m_customEndMarker.IsNull()) myContext->Remove(m_customEndMarker, Standard_False);
    TopoDS_Shape sphere = BRepPrimAPI_MakeSphere(rawPt, 4.0).Shape();
    m_customEndMarker = new AIS_Shape(sphere);
    myContext->SetColor(m_customEndMarker, Quantity_NOC_RED, Standard_False);
    myContext->Display(m_customEndMarker, Standard_False);

    // Draw "END" Label right above the Sphere!
    if (!m_customEndLabel.IsNull()) myContext->Remove(m_customEndLabel, Standard_False);
    m_customEndLabel = new AIS_TextLabel();
    m_customEndLabel->SetText(TCollection_ExtendedString("END"));
    m_customEndLabel->SetPosition(gp_Pnt(rawPt.X(), rawPt.Y(), rawPt.Z() + 15.0));
    m_customEndLabel->SetHeight(22);
    m_customEndLabel->SetColor(Quantity_NOC_RED);
    m_customEndLabel->SetFontAspect(Font_FA_Bold);
    myContext->Display(m_customEndLabel, Standard_True);

    // 🚀 THE FIX: Convert to Robot coords and update the UI box!
    gp_Trsf inverseTranslation;
    inverseTranslation.SetTranslation(gp_Vec(-myCustomOrigin.X(), -myCustomOrigin.Y(), -myCustomOrigin.Z()));
    gp_Pnt robotPt = rawPt.Transformed(inverseTranslation);

    QString xyzText = QString("X: %1 | Y: %2 | Z: %3")
                          .arg(robotPt.X(), 0, 'f', 2)
                          .arg(robotPt.Y(), 0, 'f', 2)
                          .arg(robotPt.Z(), 0, 'f', 2);

    emit customEndPointCalculated(xyzText);
    emit statusUpdate("✅ End Point Set.");
}
