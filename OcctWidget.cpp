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
#include <algorithm>


#include <Graphic3d_HorizontalTextAlignment.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <gp_Trsf.hxx>
#include <Font_FontAspect.hxx>
#include <kdl/frames.hpp>
#include <BRepTools_WireExplorer.hxx>
#include <GeomAbs_CurveType.hxx>

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


    double cylR = 2.5 * scale;
    double cylL = 60.0 * scale;
    double coneR = 7.0 * scale;
    double coneL = 18.0 * scale;

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

    myView->ChangeRenderingParams().NbMsaaSamples = 8;

    myView->ChangeRenderingParams().LineFeather = 1.0;

    // ==========================================
    // ✅ FIX 2: ISOLATE THE GRID AND VIEWCUBE
    // ==========================================
    if (myRole == MainRole) {
        Handle(AIS_ViewCube) viewCube = new AIS_ViewCube();

        viewCube->SetDrawAxes(Standard_True);

        // 🚀 THE FIX: Increase the size of the cube and its font!
        viewCube->SetSize(85);         // Increased from 55
        viewCube->SetFontHeight(16);   // Increased from 12
        viewCube->SetAxesLabels("X", "Y", "Z");

        // 🚀 THE FIX: Move to Bottom-Left and push it up above the UI!
        // Aspect_TOTP_LEFT_LOWER anchors it to the bottom-left corner.
        // Graphic3d_Vec2i(100, 200) moves it 100px right, and 200px UP so it doesn't hide behind your Cartesian box.
        Handle(Graphic3d_TransformPers) trsfPers = new Graphic3d_TransformPers(
            Graphic3d_TMF_TriedronPers,
            Aspect_TOTP_LEFT_LOWER,
            Graphic3d_Vec2i(120, 100)
            );

        viewCube->SetTransformPersistence(trsfPers);
        myContext->Display(viewCube, Standard_False);

        // ✅ ONLY draw the giant 4000x4000 room grid if this is the Main Left Panel!
        // This stops the Right Panel from doing a massive "FitAll" zoom jerk!
        drawRoomGrid();
    }
}

void OcctWidget::loadStepFile(const std::string& filePath, int ufIndex)
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
        bool isCodeLine = true;

        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();

            if (isCodeLine) {
                currentCode = line.toInt();
                isCodeLine = false;
            } else {
                if (currentCode == 0) {
                    if (line == "LINE") {
                        if (inLine) {
                            if (saveEdgeSafely(x1, y1, z1, x2, y2, z2)) edgesAdded++;
                        }
                        inLine = true;
                        x1 = y1 = z1 = x2 = y2 = z2 = 0.0;
                    } else {
                        if (inLine) {
                            if (saveEdgeSafely(x1, y1, z1, x2, y2, z2)) edgesAdded++;
                            inLine = false;
                        }
                    }
                } else if (inLine) {
                    double val = line.toDouble();
                    if (currentCode == 10) x1 = val;
                    else if (currentCode == 20) y1 = val;
                    else if (currentCode == 30) z1 = val;
                    else if (currentCode == 11) x2 = val;
                    else if (currentCode == 21) y2 = val;
                    else if (currentCode == 31) z2 = val;
                }
                isCodeLine = true;
            }
        }

        if (inLine) {
            if (saveEdgeSafely(x1, y1, z1, x2, y2, z2)) edgesAdded++;
        }

        if (edgesAdded > 0) {
            try {
                Bnd_Box boundingBox;
                BRepBndLib::Add(comp, boundingBox);
                Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
                boundingBox.Get(xMin, yMin, zMin, xMax, yMax, zMax);

                gp_Pnt center((xMin + xMax) / 2.0, (yMin + yMax) / 2.0, (zMin + zMax) / 2.0);
                gp_Trsf centerTrsf;
                centerTrsf.SetTranslation(gp_Vec(-center.X(), -center.Y(), -center.Z()));

                TopoDS_Shape centeredShape = BRepBuilderAPI_Transform(comp, centerTrsf).Shape();
                Handle(AIS_Shape) newPart = new AIS_Shape(centeredShape);

                myContext->SetColor(newPart, Quantity_NOC_GRAY75, Standard_False);
                myContext->SetMaterial(newPart, Graphic3d_NOM_ALUMINIUM, Standard_False);
                myContext->SetDisplayMode(newPart, 1, Standard_False);

                // 🚀 MULTI-TASK: Assign to the specific UserFrame map slot!
                myLoadedParts[ufIndex] = newPart;

                // 🚀 MULTI-TASK: Move to its assigned UserFrame Origin
                if (myUFOrigins.count(ufIndex)) {
                    gp_Trsf moveTrsf;
                    moveTrsf.SetTranslation(gp_Vec(myUFOrigins[ufIndex].X(), myUFOrigins[ufIndex].Y(), myUFOrigins[ufIndex].Z()));
                    myContext->SetLocation(newPart, TopLoc_Location(moveTrsf));
                }

                myContext->Display(newPart, Standard_True);
                if (myRole == OcctWidget::SideRole) { myView->FitAll(); }
                setSelectionMode(myCurrentSelectionMode);

                emit statusUpdate(QString("✅ DXF Centered & Loaded to Task %1 (%2 Lines).").arg(ufIndex + 1).arg(edgesAdded));
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

            TopoDS_Shape baseShape = XCAFDoc_ShapeTool::GetShape(aLabel);
            Bnd_Box boundingBox;
            BRepBndLib::Add(baseShape, boundingBox);
            Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
            boundingBox.Get(xMin, yMin, zMin, xMax, yMax, zMax);

            gp_Pnt center((xMin + xMax) / 2.0, (yMin + yMax) / 2.0, (zMin + zMax) / 2.0);
            gp_Trsf centerTrsf;
            centerTrsf.SetTranslation(gp_Vec(-center.X(), -center.Y(), -center.Z()));

            TopoDS_Shape centeredShape = BRepBuilderAPI_Transform(baseShape, centerTrsf).Shape();
            Handle(AIS_Shape) newPart = new AIS_Shape(centeredShape);

            myContext->SetColor(newPart, Quantity_NOC_GRAY75, Standard_False);
            myContext->SetMaterial(newPart, Graphic3d_NOM_ALUMINIUM, Standard_False);
            myContext->SetDisplayMode(newPart, 1, Standard_False);

            // 🚀 MULTI-TASK: Assign to the specific UserFrame map slot!
            myLoadedParts[ufIndex] = newPart;

            // 🚀 MULTI-TASK: Move to its assigned UserFrame Origin
            if (myUFOrigins.count(ufIndex)) {
                gp_Trsf moveTrsf;
                moveTrsf.SetTranslation(gp_Vec(myUFOrigins[ufIndex].X(), myUFOrigins[ufIndex].Y(), myUFOrigins[ufIndex].Z()));
                myContext->SetLocation(newPart, TopLoc_Location(moveTrsf));
            }

            myContext->Display(newPart, Standard_True);

            if (myRole == OcctWidget::SideRole) { myView->FitAll(); }
            myView->Redraw();
            setSelectionMode(myCurrentSelectionMode);

            emit statusUpdate(QString("✅ Workpiece Centered & Loaded to Task %1.").arg(ufIndex + 1));
        }
    }
}




void OcctWidget::resetOrigin(int ufIndex)
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
// ====================================================================
// 🚀 1. PROCESS CURRENT SELECTION (WITH MULTI-TASK MEMORY)
// ====================================================================
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

    while (myContext->MoreSelected()) {
        TopoDS_Shape shape = myContext->SelectedShape();
        Handle(AIS_InteractiveObject) selObj = myContext->SelectedInteractive();

        // 🚀 AUTOMATICALLY FIND THE PARENT USERFRAME
        int activeUfIndex = -1;
        for (auto const& [ufIdx, aisPart] : myLoadedParts) {
            if (selObj == aisPart) {
                activeUfIndex = ufIdx;
                break;
            }
        }

        // Fetch the corresponding origin (Fallback to 0,0,0 if not found)
        gp_Pnt activeOrigin = (activeUfIndex != -1 && myUFOrigins.count(activeUfIndex))
                                  ? myUFOrigins[activeUfIndex]
                                  : gp_Pnt(0,0,0);

        Handle(AIS_Shape) plottedPath = new AIS_Shape(shape);
        myContext->SetColor(plottedPath, Quantity_NOC_RED, Standard_False);
        if (shape.ShapeType() == TopAbs_FACE) myContext->SetDisplayMode(plottedPath, 1, Standard_False);
        else myContext->SetWidth(plottedPath, 3.0, Standard_False);

        myContext->Display(plottedPath, Standard_True);

        // 🚀 SAVE THE ORIGIN TO HISTORY SO REGENERATE CSV DOESN'T BREAK!
        myPathHistory.push_back({shape, plottedPath, resolution, activeOrigin});
        addedCount++;

        // Pass the ACTIVE ORIGIN into your processing functions
        switch (shape.ShapeType()) {
        case TopAbs_FACE: processFace(TopoDS::Face(shape), stringOut, resolution, activeOrigin); break;
        case TopAbs_WIRE: processWire(TopoDS::Wire(shape), stringOut, resolution, activeOrigin); break;
        case TopAbs_EDGE: processEdge(TopoDS::Edge(shape), stringOut, resolution, activeOrigin); break;
        default: break;
        }

        myContext->NextSelected();
    }
    myContext->ClearSelected(Standard_True);
    this->setProperty("selectionCount", 0); // Reset the count mathematically
    myRedoStack.clear();

    emit coordinatesExtracted(txtData);
    regenerateCSV();
    emit statusUpdate(QString("✅ Extracted %1 new path(s). Total Paths: %2").arg(addedCount).arg(myPathHistory.size()));

    // Triggers the UI to flip back to Yellow and display "COUNT: 0"
    emit selectionChanged(false);
}
void OcctWidget::regenerateCSV()
{
    if (myPathHistory.empty()) {
        return;
    }

    QFile file(myCSVPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return;
    }

    QTextStream out(&file);

    for (const auto& step : myPathHistory) {
        // 🚀 WE NOW PASS THE STORED `step.activeOrigin` BACK INTO THE MATH!
        switch (step.shape.ShapeType()) {
        case TopAbs_FACE: processFace(TopoDS::Face(step.shape), out, step.resolution, step.activeOrigin); break;
        case TopAbs_WIRE: processWire(TopoDS::Wire(step.shape), out, step.resolution, step.activeOrigin); break;
        case TopAbs_EDGE: processEdge(TopoDS::Edge(step.shape), out, step.resolution, step.activeOrigin); break;
        default: break;
        }
    }

    file.close();
}





static gp_Dir g_faceNormal(0, 0, 1);
static bool g_hasFaceNormal = false;


void OcctWidget::processFace(const TopoDS_Face& face, QTextStream& out, double resolution, const gp_Pnt& activeOrigin)
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
        processWire(wire, out, resolution, activeOrigin);
    }
    g_hasFaceNormal = false;
}

void OcctWidget::processWire(const TopoDS_Wire& wire, QTextStream& out, double resolution, const gp_Pnt& activeOrigin)
{
    gp_Dir normal = g_hasFaceNormal ? g_faceNormal : gp_Dir(0, 0, 1);

    // 🚀 REVERTED: The Tool's X-axis (Red Arrow) must point INTO the surface (-normal)
    gp_Dir x_axis(-normal.X(), -normal.Y(), -normal.Z());

    gp_Dir reference_dir(1, 0, 0);
    if (x_axis.IsParallel(reference_dir, 0.01)) {
        reference_dir = gp_Dir(0, 1, 0);
    }

    gp_Dir y_axis = reference_dir.Crossed(x_axis);
    gp_Dir z_axis = x_axis.Crossed(y_axis);

    auto writeCoordinates = [&](const gp_Pnt& point) {
        gp_Ax3 toolPos(point, z_axis, x_axis);

        gp_Trsf inverseTranslation;
        inverseTranslation.SetTranslation(gp_Vec(-activeOrigin.X(), -activeOrigin.Y(), -activeOrigin.Z()));
        toolPos.Transform(inverseTranslation);

        gp_Ax3 defaultOXY(gp_Pnt(0,0,0), gp_Dir(0,0,1), gp_Dir(1,0,0));
        gp_Trsf trsf;
        trsf.SetDisplacement(defaultOXY, toolPos);

        gp_Mat m = trsf.VectorialPart();
        KDL::Rotation kdlRot(m.Value(1,1), m.Value(1,2), m.Value(1,3),
                             m.Value(2,1), m.Value(2,2), m.Value(2,3),
                             m.Value(3,1), m.Value(3,2), m.Value(3,3));
        double rx, ry, rz;
        RobotMath::getUnifiedEulerDegrees(kdlRot, rx, ry, rz);

        out << toolPos.Location().X() << "," << toolPos.Location().Y() << "," << toolPos.Location().Z() << ","
            << rx << "," << ry << "," << rz << "\n";
    };

    BRepTools_WireExplorer explorer(wire);

    int totalEdges = 0;
    for (BRepTools_WireExplorer countExp(wire); countExp.More(); countExp.Next()) {
        totalEdges++;
    }

    int currentEdge = 0;

    for (; explorer.More(); explorer.Next()) {
        TopoDS_Edge edge = explorer.Current();
        currentEdge++;

        Standard_Real first, last;
        Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, first, last);
        if (curve.IsNull()) continue;

        BRepAdaptor_Curve adaptor(edge);

        // =======================================================
        // 🚀 SMART PEN LOGIC: LINE vs CURVE
        // =======================================================
        GeomAbs_CurveType curveType = adaptor.GetType();
        std::vector<Standard_Real> pointParams;

        if (curveType == GeomAbs_Line) {
            // Straight Line: Only Start and End points! (Saves points, 4 points for a square)
            pointParams.push_back(first);
            pointParams.push_back(last);
        } else {
            // Curve: High resolution points for smooth arcs and circles!
            GCPnts_UniformAbscissa discretizer(adaptor, resolution, first, last);
            if (discretizer.IsDone()) {
                for (int i = 1; i <= discretizer.NbPoints(); ++i) {
                    pointParams.push_back(discretizer.Parameter(i));
                }
            }
        }

        // =======================================================
        // 🚀 REVERSE FIX: PREVENT DIAGONAL JUMPS ON SQUARES!
        // =======================================================
        if (edge.Orientation() == TopAbs_REVERSED) {
            std::reverse(pointParams.begin(), pointParams.end());
        }

        // =======================================================
        // Process the Generated Points
        // =======================================================
        for (size_t i = 0; i < pointParams.size(); ++i) {

            // Prevent duplicate coordinates at corners
            if (currentEdge > 1 && i == 0) continue;

            Standard_Real param = pointParams[i];
            gp_Pnt pt;
            gp_Vec tangentVec;
            adaptor.D1(param, pt, tangentVec);

            if (!m_isFirstPointFound) {
                gp_Ax3 startPos(pt, z_axis, x_axis);
                drawStartMarker(startPos.Location());
                m_isFirstPointFound = true;
            }

            // 🚀 Z-HOP APPROACH: 10mm
            if (currentEdge == 1 && i == 0) {
                gp_Pnt approachPt = pt.Translated(gp_Vec(normal) * 30.0);
                writeCoordinates(approachPt);
            }

            // 📍 ACTUAL SURFACE POINT
            writeCoordinates(pt);

            // 🚀 Z-HOP RETRACT: 10mm
            if (currentEdge == totalEdges && i == (pointParams.size() - 1)) {
                gp_Pnt retractPt = pt.Translated(gp_Vec(normal) * 30.0);
                writeCoordinates(retractPt);
            }
        }
    }
}

void OcctWidget::processEdge(const TopoDS_Edge& edge, QTextStream& out, double resolution, const gp_Pnt& activeOrigin)
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

    // =======================================================
    // 🚀 SMART PEN LOGIC: LINE vs CURVE
    // =======================================================
    GeomAbs_CurveType curveType = adaptor.GetType();
    std::vector<Standard_Real> pointParams;

    if (curveType == GeomAbs_Line) {
        // Straight Line: Only Start and End points
        pointParams.push_back(first);
        pointParams.push_back(last);
    } else {
        // Curve/Arc: High resolution points
        GCPnts_UniformAbscissa discretizer(adaptor, resolution, first, last);
        if (discretizer.IsDone()) {
            for (int i = 1; i <= discretizer.NbPoints(); ++i) {
                pointParams.push_back(discretizer.Parameter(i));
            }
        }
    }

    // =======================================================
    // 🚀 REVERSE FIX: PREVENT DIAGONAL JUMPS!
    // =======================================================
    if (edge.Orientation() == TopAbs_REVERSED) {
        std::reverse(pointParams.begin(), pointParams.end());
    }

    gp_Dir normal = g_hasFaceNormal ? g_faceNormal : gp_Dir(0, 0, 1);

    // 🚀 REVERTED: The Tool's X-axis (Red Arrow) must point INTO the surface (-normal)
    gp_Dir x_axis(-normal.X(), -normal.Y(), -normal.Z());

    gp_Dir reference_dir(1, 0, 0);
    if (x_axis.IsParallel(reference_dir, 0.01)) {
        reference_dir = gp_Dir(0, 1, 0);
    }

    gp_Dir y_axis = reference_dir.Crossed(x_axis);
    gp_Dir z_axis = x_axis.Crossed(y_axis);

    auto writeCoordinates = [&](const gp_Pnt& point) {
        gp_Ax3 toolPos(point, z_axis, x_axis);

        gp_Trsf inverseTranslation;
        inverseTranslation.SetTranslation(gp_Vec(-activeOrigin.X(), -activeOrigin.Y(), -activeOrigin.Z()));
        toolPos.Transform(inverseTranslation);

        gp_Ax3 defaultOXY(gp_Pnt(0,0,0), gp_Dir(0,0,1), gp_Dir(1,0,0));
        gp_Trsf trsf;
        trsf.SetDisplacement(defaultOXY, toolPos);

        gp_Mat m = trsf.VectorialPart();
        KDL::Rotation kdlRot(m.Value(1,1), m.Value(1,2), m.Value(1,3),
                             m.Value(2,1), m.Value(2,2), m.Value(2,3),
                             m.Value(3,1), m.Value(3,2), m.Value(3,3));
        double rx, ry, rz;
        RobotMath::getUnifiedEulerDegrees(kdlRot, rx, ry, rz);

        out << toolPos.Location().X() << "," << toolPos.Location().Y() << "," << toolPos.Location().Z() << ","
            << rx << "," << ry << "," << rz << "\n";
    };

    for (size_t i = 0; i < pointParams.size(); ++i) {
        Standard_Real param = pointParams[i];
        gp_Pnt pt;
        gp_Vec tangentVec;
        adaptor.D1(param, pt, tangentVec);

        if (!m_isFirstPointFound) {
            if (!isTrimmedEdge) {
                gp_Ax3 startPos(pt, z_axis, x_axis);
                drawStartMarker(startPos.Location());
            }
            m_isFirstPointFound = true;
        }

        // 🚀 Z-HOP APPROACH: 10mm
        if (i == 0) {
            gp_Pnt approachPt = pt.Translated(gp_Vec(normal) * 30.0);
            writeCoordinates(approachPt);
        }

        // 📍 ACTUAL SURFACE POINT
        writeCoordinates(pt);

        // 🚀 Z-HOP RETRACT: 10mm
        if (i == (pointParams.size() - 1)) {
            gp_Pnt retractPt = pt.Translated(gp_Vec(normal) * 30.0);
            writeCoordinates(retractPt);
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
        myContext->SelectDetected(AIS_SelectionScheme_XOR);
        myView->Redraw();

        // 🚀 NEW: Count the total number of selected shapes
        myContext->InitSelected();
        int selCount = 0;
        while(myContext->MoreSelected()) {
            selCount++;
            myContext->NextSelected();
        }

        // Save the count into a dynamic property so RightPanel can read it
        this->setProperty("selectionCount", selCount);

        // Reset the iterator for downstream logic
        myContext->InitSelected();

        // ==========================================================
        // 🚀 THE FIX: Check MoreSelected() BEFORE checking for Shapes
        // ==========================================================
        if (myContext->MoreSelected()) {

            Handle(AIS_InteractiveObject) selObj = myContext->SelectedInteractive();

            // ✅ 1. THE SHIELD: Intercept ViewCube clicks and ROTATE the Camera
            if (!selObj.IsNull() && selObj->DynamicType() == STANDARD_TYPE(AIS_ViewCube)) {

                // Get the Owner to figure out exactly which face (Top, Front, Corner) was clicked
                Handle(AIS_ViewCubeOwner) vcOwner = Handle(AIS_ViewCubeOwner)::DownCast(myContext->SelectedOwner());
                if (!vcOwner.IsNull()) {
                    // Snap the camera to that face and re-center the view!
                    myView->SetProj(vcOwner->MainOrientation());
                    myView->FitAll();
                    myView->Redraw();
                }

                // Clear the selection so the cube doesn't stay permanently highlighted
                myContext->ClearSelected(Standard_True);
                return; // 🛑 Stop here! Do not process this as a CAD shape.
            }

            // ✅ 2. Process regular CAD shape selections
            if (myContext->HasSelectedShape()) {

                if (myIsSettingOriginMode) {
                    TopoDS_Shape selectedShape = myContext->SelectedShape();
                    Bnd_Box boundingBox;
                    BRepBndLib::Add(selectedShape, boundingBox);
                    Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
                    boundingBox.Get(xMin, yMin, zMin, xMax, yMax, zMax);
                    gp_Pnt newOriginSnap((xMin + xMax) / 2.0, (yMin + yMax) / 2.0, (zMin + zMax) / 2.0);

                    QString msg = QString("Do you want to set the new Robot Origin here?\n\nX: %1\nY: %2\nZ: %3")
                                      .arg(newOriginSnap.X(), 0, 'f', 2).arg(newOriginSnap.Y(), 0, 'f', 2).arg(newOriginSnap.Z(), 0, 'f', 2);

                    if (QMessageBox::question(this, "Confirm New Origin", msg) == QMessageBox::Yes) {
                        int activeUf = 0; // Or whatever logic you use to select the current Task/UF
                        myUFOrigins[activeUf] = newOriginSnap;
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

                if (myRole == MainRole) {
                    TopoDS_Shape selectedShape = myContext->SelectedShape();
                    emit partSelectedForIsolation(selectedShape);
                    myContext->ClearSelected(Standard_True);
                }
                else if (myRole == SideRole) {
                    emit selectionChanged(true); // Triggers RightPanel to update UI to > 0
                }
            }
        } else {
            // Selection is completely empty
            if (myRole == SideRole) {
                emit selectionChanged(false); // Triggers RightPanel to update UI back to 0
            }
        }
    }
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
void OcctWidget::offsetWorkpiece(int ufIndex, double dx, double dy, double dz)
{
    if (!myLoadedParts.count(ufIndex)) {
        emit statusUpdate("⚠️ Load a part first before moving it.");
        return;
    }

    // Create a 3D Translation Vector
    gp_Trsf transform;
    transform.SetTranslation(gp_Vec(dx, dy, dz));
    TopLoc_Location loc(transform);

    // Apply the offset to the specific part
    myContext->SetLocation(myLoadedParts[ufIndex], loc);
    myContext->UpdateCurrentViewer();

    emit statusUpdate(QString("📏 Part Calibrated to Robot Base -> X:%1, Y:%2, Z:%3").arg(dx).arg(dy).arg(dz));
}



// ==========================================
// 1. TRIGGER THE ASYNC LOADER
// ==========================================
void OcctWidget::loadDefaultRobot()
{
    if (myView.IsNull()) initOCCT();

    myCurrentLoadIndex = 0;
    m_activeSession = m_loadSessionId; // 🚀 LOCK IN THE CURRENT SESSION

    emit statusUpdate("⏳ Loading STL Robot Base (link0)...");
    QTimer::singleShot(50, this, &OcctWidget::loadNextRobotLink);
}

void OcctWidget::loadNextRobotLink()
{
    // 🚀 IF A NEW ROBOT WAS LOADED, ABORT THIS OLD LOOP!
    if (m_activeSession != m_loadSessionId) return;

    if (myCurrentLoadIndex > 6) {
        myBaseTriad = createThickTriad(1.5);
        myTipTriad = createThickTriad(1.2);

        myContext->SetDisplayMode(myBaseTriad, 1, Standard_False);
        myContext->SetDisplayMode(myTipTriad, 1, Standard_False);

        // 🚀 THE FIX: Removed the -90 degree rotation!
        // Applying a blank transform keeps X(Red), Y(Green), and Z(Blue) perfectly aligned with the world.
        gp_Trsf identityTrsf;
        myContext->SetLocation(myBaseTriad, TopLoc_Location(identityTrsf));

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

    // 2. Setup the file path using the Dynamic Prefix!
    QString folderPath = m_robotFolderPath;
    if (!folderPath.endsWith("/")) folderPath += "/";

    // 🚀 USE THE PREFIX VARIABLE HERE:
    QString fileName = folderPath + m_robotLinkPrefix + QString::number(myCurrentLoadIndex) + ".stl";

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

    }

    // 3. Increment the tracker to the next part
    myCurrentLoadIndex++;

    // 4. Update the UI text
    emit statusUpdate(QString("⏳ Loading Raw Mesh (link%1.stl)...").arg(myCurrentLoadIndex - 1));

    QTimer::singleShot(50, this, &OcctWidget::loadNextRobotLink);
}






void OcctWidget::updateRobotPosture(double j1, double j2, double j3, double j4, double j5, double j6)
{
    if (myRobotLinks.empty()) return;

    gp_Trsf baseTrsf;
    baseTrsf.SetScale(gp_Pnt(0,0,0), 1000.0);

    gp_Pnt orig1, orig2, orig3, orig4, orig5, orig6;
    gp_Dir axis1, axis2, axis3, axis4, axis5, axis6;
    gp_Trsf moveToFlange;

    if (m_isCobot) {
        orig1 = gp_Pnt(0.0, 0.0, 0.0);
        orig2 = gp_Pnt(m_rob_bx, 0.0, m_rob_bz);
        orig3 = gp_Pnt(m_rob_bx, 0.0, m_rob_bz + m_rob_az);
        orig4 = gp_Pnt(m_rob_bx, 0.0, m_rob_bz + m_rob_az + m_rob_ez);
        orig5 = gp_Pnt(m_rob_bx, m_rob_fx, m_rob_bz + m_rob_az + m_rob_ez);
        orig6 = gp_Pnt(m_rob_bx, m_rob_fx, m_rob_bz + m_rob_az + m_rob_ez + m_rob_wx);

        axis1 = gp_Dir(0, 0, 1);
        axis2 = gp_Dir(0, 1, 0);
        axis3 = gp_Dir(0, 1, 0);
        axis4 = gp_Dir(0, 1, 0);
        axis5 = gp_Dir(0, 0, 1);
        axis6 = gp_Dir(0, 1, 0);

        // 🚀 THE FIX: Twist -90 on X so the Blue Arrow (Z) points OUT!
        gp_Trsf flangeTwist;
        flangeTwist.SetRotation(gp_Ax1(gp_Pnt(0,0,0), gp_Dir(1,0,0)), -M_PI / 2.0);

        gp_Trsf flangeTranslation;
        flangeTranslation.SetTranslation(gp_Vec(m_rob_bx, m_rob_fx + m_rob_fy, m_rob_bz + m_rob_az + m_rob_ez + m_rob_wx));

        moveToFlange = flangeTranslation * flangeTwist;
    } else {
        // 🚀 INDUSTRIAL KINEMATICS
        orig1 = gp_Pnt(0.0, 0.0, 0.0);
        orig2 = gp_Pnt(m_rob_bx, 0.0, m_rob_bz);
        orig3 = gp_Pnt(m_rob_bx, 0.0, m_rob_bz + m_rob_az);
        orig4 = gp_Pnt(m_rob_bx, 0.0, m_rob_bz + m_rob_az + m_rob_ez);
        orig5 = gp_Pnt(m_rob_bx + m_rob_fx, 0.0, m_rob_bz + m_rob_az + m_rob_ez);
        orig6 = gp_Pnt(orig5.X(), orig5.Y(), orig5.Z());

        axis1 = gp_Dir(0, 0, 1);
        axis2 = gp_Dir(0, 1, 0);
        axis3 = gp_Dir(0, 1, 0);
        axis4 = gp_Dir(1, 0, 0);
        axis5 = gp_Dir(0, 1, 0);
        axis6 = gp_Dir(1, 0, 0);

        moveToFlange.SetTranslation(gp_Vec(m_rob_bx + m_rob_fx + m_rob_wx, 0.0, m_rob_bz + m_rob_az + m_rob_ez));
    }

    // Apply Rotations
    gp_Trsf R1, R2, R3, R4, R5, R6;
    R1.SetRotation(gp_Ax1(orig1, axis1), j1);
    R2.SetRotation(gp_Ax1(orig2, axis2), j2);
    R3.SetRotation(gp_Ax1(orig3, axis3), j3);
    R4.SetRotation(gp_Ax1(orig4, axis4), j4);
    R5.SetRotation(gp_Ax1(orig5, axis5), j5);
    R6.SetRotation(gp_Ax1(orig6, axis6), j6);

    gp_Trsf accum1 = R1;
    gp_Trsf accum2 = R1 * R2;
    gp_Trsf accum3 = R1 * R2 * R3;
    gp_Trsf accum4 = R1 * R2 * R3 * R4;
    gp_Trsf accum5 = R1 * R2 * R3 * R4 * R5;
    gp_Trsf accum6 = R1 * R2 * R3 * R4 * R5 * R6;

    if (myRobotLinks.size() > 0) myContext->SetLocation(myRobotLinks[0], TopLoc_Location(baseTrsf));
    if (myRobotLinks.size() > 1) myContext->SetLocation(myRobotLinks[1], TopLoc_Location(accum1 * baseTrsf));
    if (myRobotLinks.size() > 2) myContext->SetLocation(myRobotLinks[2], TopLoc_Location(accum2 * baseTrsf));
    if (myRobotLinks.size() > 3) myContext->SetLocation(myRobotLinks[3], TopLoc_Location(accum3 * baseTrsf));
    if (myRobotLinks.size() > 4) myContext->SetLocation(myRobotLinks[4], TopLoc_Location(accum4 * baseTrsf));
    if (myRobotLinks.size() > 5) myContext->SetLocation(myRobotLinks[5], TopLoc_Location(accum5 * baseTrsf));
    if (myRobotLinks.size() > 6) myContext->SetLocation(myRobotLinks[6], TopLoc_Location(accum6 * baseTrsf));

    if (!myTipTriad.IsNull()) {
        gp_Trsf flangeTrsf = accum6 * moveToFlange;
        myLastTipTrsf = flangeTrsf;

        if (!myToolShape.IsNull()) {
            myContext->SetLocation(myToolShape, TopLoc_Location(flangeTrsf));
        }

        gp_Trsf toolOffsetTrsf;
        toolOffsetTrsf.SetTranslation(gp_Vec(m_toolOffsetX, m_toolOffsetY, m_toolOffsetZ));
        gp_Trsf tcpTrsf = flangeTrsf * toolOffsetTrsf;

        myContext->SetLocation(myTipTriad, TopLoc_Location(tcpTrsf));

        // ========================================================
        // ✅ DRAW THE LIVE TRAJECTORY TRAIL FROM THE TCP
        // ========================================================
        gp_Pnt currentTCP = gp_Pnt(0,0,0).Transformed(tcpTrsf);

        if (myTrajectoryPoints.empty() || myTrajectoryPoints.back().Distance(currentTCP) > 1.0) {
            myTrajectoryPoints.push_back(currentTCP);

            if (myTrajectoryPoints.size() > 5000) {
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



QString OcctWidget::getOriginText(int ufIndex) const {
    if (!myUFOrigins.count(ufIndex)) return "X: 0.000 | Y: 0.000 | Z: 0.000";

    gp_Pnt origin = myUFOrigins.at(ufIndex);
    return QString("X: %1 | Y: %2 | Z: %3")
        .arg(origin.X(), 0, 'f', 3)
        .arg(origin.Y(), 0, 'f', 3)
        .arg(origin.Z(), 0, 'f', 3);
}

void OcctWidget::drawRoomGrid()
{
    // Defined boundaries (Expanded to 7000)
    double minX = -7000.0, maxX = 7000.0;
    double minY = -7000.0, maxY = 7000.0;
    double minZ = 0.0, maxZ = 3000.0;

    double step = 500.0; // Physical grid lines every 100mm
    int labelStep = 500.0; // Text labels every 500mm to prevent overlapping text

    TopoDS_Compound floorComp, backWallComp, leftWallComp;
    BRep_Builder builder;
    builder.MakeCompound(floorComp);
    builder.MakeCompound(backWallComp);
    builder.MakeCompound(leftWallComp);

    // 1. FLOOR (Now Green)
    for (double x = minX; x <= maxX; x += step)
        builder.Add(floorComp, BRepBuilderAPI_MakeEdge(gp_Pnt(x, minY, minZ), gp_Pnt(x, maxY, minZ)));
    for (double y = minY; y <= maxY; y += step)
        builder.Add(floorComp, BRepBuilderAPI_MakeEdge(gp_Pnt(minX, y, minZ), gp_Pnt(maxX, y, minZ)));

    // ---------------------------------------------------------
    // 2. BACK WALL (Blue Wall - Behind the robot at X = minX)
    // ---------------------------------------------------------
    for (double y = minY; y <= maxY; y += step)
        builder.Add(backWallComp, BRepBuilderAPI_MakeEdge(gp_Pnt(minX, y, minZ), gp_Pnt(minX, y, maxZ)));
    for (double z = minZ; z <= maxZ; z += step)
        builder.Add(backWallComp, BRepBuilderAPI_MakeEdge(gp_Pnt(minX, minY, z), gp_Pnt(minX, maxY, z)));

    // ---------------------------------------------------------
    // 3. SIDE WALL (Now Red) - Moved to Y = minY (Right side of the robot)
    // ---------------------------------------------------------
    for (double x = minX; x <= maxX; x += step)
        builder.Add(leftWallComp, BRepBuilderAPI_MakeEdge(gp_Pnt(x, minY, minZ), gp_Pnt(x, minY, maxZ)));
    for (double z = minZ; z <= maxZ; z += step)
        builder.Add(leftWallComp, BRepBuilderAPI_MakeEdge(gp_Pnt(minX, minY, z), gp_Pnt(maxX, minY, z)));

    Handle(AIS_Shape) aisFloor = new AIS_Shape(floorComp);
    Handle(AIS_Shape) aisBack = new AIS_Shape(backWallComp);
    Handle(AIS_Shape) aisLeft = new AIS_Shape(leftWallComp);

    // ==========================================
    // ✅ COLORS SETTING (SWAPPED FLOOR AND SIDE WALL)
    // ==========================================
    Quantity_Color greenFloor(Quantity_NOC_SEAGREEN);
    Quantity_Color redSideWall(Quantity_NOC_INDIANRED1);

    myContext->SetColor(aisFloor, greenFloor, Standard_False);
    myContext->SetColor(aisBack, Quantity_NOC_STEELBLUE, Standard_False);
    myContext->SetColor(aisLeft, redSideWall, Standard_False);

    // 1080p-க்கு 1.5 Thickness வைத்தால் கச்சிதமாகத் தெரியும்.
    myContext->SetWidth(aisFloor, 1.5, Standard_False);
    myContext->SetWidth(aisBack, 1.5, Standard_False);
    myContext->SetWidth(aisLeft, 1.5, Standard_False);

    myContext->Display(aisFloor, Standard_False);
    myContext->Display(aisBack, Standard_False);
    myContext->Display(aisLeft, Standard_False);
    myContext->Deactivate(aisFloor);
    myContext->Deactivate(aisBack);
    myContext->Deactivate(aisLeft);

    // 🚀 THE FIX: Increased base text size significantly from 60 to 150
    int textSize = 150;

    // ==========================================
    // 🚀 RED LABELS (X-Axis / Front-Back)
    // ==========================================
    for (int x = -7000; x <= 7000; x += labelStep) {
        Handle(AIS_TextLabel) xLabel = new AIS_TextLabel();

        if (x == 0) {
            xLabel->SetText(TCollection_ExtendedString("X: 0")); // Display X at 0
        } else {
            xLabel->SetText(TCollection_ExtendedString(x));
        }

        xLabel->SetPosition(gp_Pnt(x, maxY + 50.0, 0));
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
    for (int y = -7000; y <= 7000; y += labelStep) {
        Handle(AIS_TextLabel) yLabel = new AIS_TextLabel();

        if (y == 0) {
            yLabel->SetText(TCollection_ExtendedString("Y: 0")); // Display Y at 0
        } else {
            yLabel->SetText(TCollection_ExtendedString(y));
        }

        yLabel->SetPosition(gp_Pnt(maxX + 50.0, y, 0));
        yLabel->SetHeight(textSize);
        yLabel->SetColor(Quantity_NOC_DARKGREEN);
        yLabel->SetFontAspect(Font_FA_Bold);
        yLabel->SetZoomable(Standard_True);
        myContext->Display(yLabel, Standard_False);
        myContext->Deactivate(yLabel);
    }

    // ==========================================
    // 🚀 Z-AXIS LABELS (Blue)
    // ==========================================
    for (int z = 0; z <= 3000; z += 500) {
        Handle(AIS_TextLabel) zLabel = new AIS_TextLabel();

        if (z == 0) {
            zLabel->SetText(TCollection_ExtendedString("Z: 0")); // Display Z at 0
        } else {
            zLabel->SetText(TCollection_ExtendedString(z));
        }

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

    // 🚀 THE FIX: Increased Title Size (from 60 to 150) & Added Boldness
    titleLabel->SetHeight(150);
    titleLabel->SetFontAspect(Font_FA_Bold);

    titleLabel->SetColor(Quantity_NOC_BLACK);
    titleLabel->SetZoomable(Standard_True);
    titleLabel->SetOrientation3D(gp_Ax2(gp_Pnt(minX, 0.0, 1200), gp_Dir(1, 0, 0), gp_Dir(0, 1, 0)));
    titleLabel->SetHJustification(Graphic3d_HTA_CENTER);
    myContext->Display(titleLabel, Standard_False);
    myContext->Deactivate(titleLabel);

    Handle(AIS_TextLabel) subLabel = new AIS_TextLabel();
    subLabel->SetText(TCollection_ExtendedString("R O B O T I C S"));
    subLabel->SetPosition(gp_Pnt(minX + 1.0, 0.0, 900));

    // 🚀 THE FIX: Increased Subtitle Size (from 30 to 80) & Added Boldness
    subLabel->SetHeight(80);
    subLabel->SetFontAspect(Font_FA_Bold);

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
void OcctWidget::setUserFrameOrigin(int ufIndex, double ui_x, double ui_y, double ui_z)
{
    if (myContext.IsNull()) return;

    myCustomOrigin = gp_Pnt(ui_x, ui_y, ui_z);

    // 1. RECOVER THE ACTIVE ROTATION
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

    // 2. Apply it to the 3D Part (Using the Map!)
    if (myLoadedParts.count(ufIndex)) {
        myContext->SetLocation(myLoadedParts[ufIndex], TopLoc_Location(finalTrsf));
    }

    // 3. Apply it to the Target Marker Triad (Using the Map!)
    if (!myUserFrameMarkers.count(ufIndex)) {
        myUserFrameMarkers[ufIndex] = createThickTriad(1.3);
        myContext->SetDisplayMode(myUserFrameMarkers[ufIndex], 1, Standard_False);
        myContext->Display(myUserFrameMarkers[ufIndex], Standard_False);
    }

    myContext->SetLocation(myUserFrameMarkers[ufIndex], TopLoc_Location(finalTrsf));

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




void OcctWidget::transformLoadedPart(int ufIndex, double dx, double dy, double dz, double rx, double ry, double rz)
{
    // Ensure the specific Task (UserFrame) actually has a part loaded
    if (!myLoadedParts.count(ufIndex)) return;

    // Save current UserFrame origin
    myUFOrigins[ufIndex] = gp_Pnt(dx, dy, dz);

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

    // Apply to the specific loaded part
    myContext->SetLocation(myLoadedParts[ufIndex], TopLoc_Location(finalTrsf));

    // Apply to the specific User Frame Marker (Arrows) if it exists
    if (myUserFrameMarkers.count(ufIndex)) {
        myContext->SetLocation(myUserFrameMarkers[ufIndex], TopLoc_Location(finalTrsf));
    }

    myContext->UpdateCurrentViewer();
}

// ====================================================================
// 🚀 NEW: ONE-CLICK FULL SHAPE EXTRACTION (SORTED & CONTINUOUS)
// ====================================================================
void OcctWidget::processAllEdges(double resolution, int ufIndex)
{
    if (myContext.IsNull() || myLoadedParts.empty()) {
        emit statusUpdate("⚠️ Load a part first!");
        return;
    }

    clearSelections();
    m_isFirstPointFound = false;
    int addedCount = 0;

    QString txtData;
    QTextStream stringOut(&txtData);

    // 🚀 Find the target part! If -1, default to the first available part.
    int targetUf = (ufIndex != -1) ? ufIndex : myLoadedParts.begin()->first;

    if (!myLoadedParts.count(targetUf)) {
        emit statusUpdate("⚠️ Error: Target Task has no loaded part.");
        return;
    }

    Handle(AIS_Shape) aisShape = myLoadedParts[targetUf];
    if (aisShape.IsNull()) {
        emit statusUpdate("⚠️ Error: The loaded part is not a valid CAD shape.");
        return;
    }

    // 🚀 Fetch the active origin for this specific Task
    gp_Pnt activeOrigin = myUFOrigins.count(targetUf) ? myUFOrigins[targetUf] : gp_Pnt(0,0,0);

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

        // 🚀 SAVE ORIGIN TO HISTORY (So CSV regeneration works later)
        myPathHistory.push_back({sortedWire, plottedPath, resolution, activeOrigin});
        addedCount++;

        // 4. Send the SORTED loop to the point generator (passing active origin)
        processWire(sortedWire, stringOut, resolution, activeOrigin);
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
// ==========================================================
// 🚀 DYNAMIC ROBOT HOT-SWAP
// ==========================================================
// ==========================================================
// 🚀 DYNAMIC ROBOT HOT-SWAP
// ==========================================================
void OcctWidget::reloadRobot(const QString& folderPath, const QString& linkPrefix, double bx, double bz, double az, double ez, double fx, double wx, double fy, bool isCobot)
{
    if (myContext.IsNull()) return;

    m_loadSessionId++; // 🚀 KILLS ANY PREVIOUS ASYNC LOOPS
    myCurrentLoadIndex = -1;
    m_isCobot = isCobot;

    // Save new parameters locally
    m_robotFolderPath = folderPath;
    m_robotLinkPrefix = linkPrefix;
    m_rob_bx = bx; m_rob_bz = bz; m_rob_az = az;
    m_rob_ez = ez; m_rob_fx = fx; m_rob_wx = wx; m_rob_fy = fy;

    for (auto& link : myRobotLinks) {
        if (!link.IsNull()) {
            myContext->Remove(link, Standard_False);
            link.Nullify();
        }
    }
    myRobotLinks.clear();

    // 3. Remove Triads (Base and Tool TCP arrows)
    if (!myBaseTriad.IsNull()) {
        myContext->Remove(myBaseTriad, Standard_False);
        myBaseTriad.Nullify();
    }
    if (!myTipTriad.IsNull()) {
        myContext->Remove(myTipTriad, Standard_False);
        myTipTriad.Nullify();
    }

    // 4. Temporarily detach the tool (if any) so it doesn't float in space
    if (!myToolShape.IsNull()) {
        myContext->Remove(myToolShape, Standard_False);
        // We do not nullify myToolShape because it can be re-attached later
    }

    myContext->UpdateCurrentViewer();

    // 5. Update the path variable
    m_robotFolderPath = folderPath;

    // 6. Trigger the async loader loop to build the new robot
    loadDefaultRobot();
}

// ----------------------------------------------------
// 1. SET USER FRAME STATE (ACTIVATING/DEACTIVATING TASKS)
// ----------------------------------------------------
// ----------------------------------------------------
// 1. SET USER FRAME STATE (ACTIVATING/DEACTIVATING TASKS)
// ----------------------------------------------------
void OcctWidget::setUserFrameState(int ufIndex, bool isActive, double ui_x, double ui_y, double ui_z)
{
    if (myContext.IsNull()) return;

    if (!isActive) {
        if (myUserFrameMarkers.count(ufIndex)) {
            myContext->Remove(myUserFrameMarkers[ufIndex], Standard_False);
            myUserFrameMarkers.erase(ufIndex);
        }
        clearLoadedPart(ufIndex);
    } else {
        // 1. Save the new origin mathematically
        myUFOrigins[ufIndex] = gp_Pnt(ui_x, ui_y, ui_z);

        // 2. Create a default pure translation (used if no part exists yet)
        gp_Trsf pureTranslation;
        pureTranslation.SetTranslation(gp_Vec(ui_x, ui_y, ui_z));

        // 3. Create the 3D Triad Marker if it doesn't exist
        if (!myUserFrameMarkers.count(ufIndex)) {
            Handle(AIS_ColoredShape) marker = createThickTriad(1.3);
            myContext->SetDisplayMode(marker, 1, Standard_False);
            myUserFrameMarkers[ufIndex] = marker;
            myContext->Display(marker, Standard_False);
        }

        // 🚀 THE FIX: Preserve Rotation if the part exists!
        if (myLoadedParts.count(ufIndex)) {
            // Get the current matrix (which includes the user's Rx, Ry, Rz from the Apply button)
            gp_Trsf currentTrsf = myContext->Location(myLoadedParts[ufIndex]).Transformation();

            // ONLY replace the XYZ position, leaving the rotation completely untouched
            currentTrsf.SetTranslationPart(gp_Vec(ui_x, ui_y, ui_z));

            // Apply the safe matrix to both the part and the marker
            myContext->SetLocation(myLoadedParts[ufIndex], TopLoc_Location(currentTrsf));
            myContext->SetLocation(myUserFrameMarkers[ufIndex], TopLoc_Location(currentTrsf));
        } else {
            // If no part is loaded, just move the marker to the XYZ
            myContext->SetLocation(myUserFrameMarkers[ufIndex], TopLoc_Location(pureTranslation));
        }
    }
    myContext->UpdateCurrentViewer();
}

// ----------------------------------------------------
// 2. CLEAR A SPECIFIC PART
// ----------------------------------------------------
void OcctWidget::clearLoadedPart(int ufIndex)
{
    // Check if the map contains a part for this specific Task/ufIndex
    if (myLoadedParts.count(ufIndex)) {
        // Remove it from the 3D viewer
        myContext->Remove(myLoadedParts[ufIndex], Standard_True);

        // Erase it from the map memory
        myLoadedParts.erase(ufIndex);

        emit statusUpdate("🗑️ File cleared from view.");
        myView->Redraw();
    }
}