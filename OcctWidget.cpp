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
#include <gp_Quaternion.hxx>
#include <gp_EulerSequence.hxx>
#include <BRepLProp_SLProps.hxx>
#include <Font_FontAspect.hxx>
#include <BRepTools.hxx>
#include <Geom_Surface.hxx>


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

// ✅ ADD THE MISSING GRID HEADERS HERE:
#include <TopoDS_Compound.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <cmath>


OcctWidget::OcctWidget(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_DontCreateNativeAncestors); // ✅ MUST BE HERE

    setMouseTracking(true);
    myCSVPath = "/home/texsonics/Videos/extracted_paths.csv";
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
    // 🚀 ULTIMATE SAFE DXF READER (With Try-Catch Crash Shield)
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

        // 🛡️ ULTIMATE SHIELD LAMBDA
        auto saveEdgeSafely = [&](double _x1, double _y1, double _z1, double _x2, double _y2, double _z2) {
            gp_Pnt p1(_x1, _y1, _z1);
            gp_Pnt p2(_x2, _y2, _z2);

            // 1. Strict Distance Check (0.01 mm க்கும் குறைவான கோடுகளை நிராகரித்துவிடும்)
            if (p1.Distance(p2) < 0.01) return false;

            // 2. The Try-Catch Shield (க்ராஷ் ஆவதைத் தடுக்கும் கவசம்)
            try {
                BRepBuilderAPI_MakeEdge edgeMaker(p1, p2);
                if (edgeMaker.IsDone()) {
                    builder.Add(comp, edgeMaker.Shape());
                    return true;
                }
            } catch (...) {
                // OCCT க்ராஷ் ஆக முயன்றால், அதை சத்தமில்லாமல் தடுத்துவிடும்!
                return false;
            }
            return false;
        };

        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            bool isCode;
            int code = line.toInt(&isCode);

            if (isCode && line.length() < 5) {
                currentCode = code;
            } else {
                if (line == "LINE") {
                    if (inLine) {
                        if (saveEdgeSafely(x1, y1, z1, x2, y2, z2)) edgesAdded++;
                    }
                    inLine = true;
                    x1 = y1 = z1 = x2 = y2 = z2 = 0.0; // Reset
                } else if (inLine) {
                    double val = line.toDouble();
                    if (currentCode == 10) x1 = val;
                    else if (currentCode == 20) y1 = val;
                    else if (currentCode == 30) z1 = val;
                    else if (currentCode == 11) x2 = val;
                    else if (currentCode == 21) y2 = val;
                    else if (currentCode == 31) z2 = val;
                }
            }
        }

        if (inLine) {
            if (saveEdgeSafely(x1, y1, z1, x2, y2, z2)) edgesAdded++;
        }
        file.close();

        if (edgesAdded > 0) {
            // 3. Display Shield (வரையும்போது க்ராஷ் ஆனால் தடுப்பதற்கு)
            try {
                myLoadedPart = new AIS_Shape(comp);
                myContext->SetColor(myLoadedPart, Quantity_NOC_CYAN1, Standard_False);
                myContext->SetWidth(myLoadedPart, 3.0, Standard_False);
                myContext->Display(myLoadedPart, Standard_True);

                myCustomOrigin = gp_Pnt(0.0, 0.0, 0.0);
                offsetWorkpiece(0.0, 0.0, 0.0);

                if (myRole == OcctWidget::SideRole) {
                    myView->FitAll();
                    myView->Redraw();
                } else {
                    myView->Redraw();
                }

                setSelectionMode(myCurrentSelectionMode);
                emit statusUpdate(QString("✅ DXF Loaded Successfully (%1 Valid Lines).").arg(edgesAdded));
            } catch (...) {
                emit statusUpdate("❌ Error: DXF contains severely corrupted geometry.");
            }
        } else {
            emit statusUpdate("❌ Error: No valid lines found in DXF.");
        }
        return;
    }

    // =======================================================
    // EXISTING STEP READER CODE
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
            myLoadedPart = new XCAFPrs_AISObject(aLabel);
            myContext->SetDisplayMode(myLoadedPart, 1, Standard_False);
            myContext->Display(myLoadedPart, Standard_True);

            myCustomOrigin = gp_Pnt(0.0, 0.0, 0.0);
            offsetWorkpiece(0.0, -800.0, 600.0);

            if (myRole == OcctWidget::SideRole) {
                myView->FitAll();
                myView->Redraw();
            } else {
                myView->Redraw();
            }
            setSelectionMode(myCurrentSelectionMode);
            emit statusUpdate("✅ Workpiece Loaded (Solid & Colored).");
        }
    } else {
        emit statusUpdate("❌ Error: Failed to load STEP file via XDE.");
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

    myPathHistory.clear();
    myRedoStack.clear();
    myContext->UpdateCurrentViewer();

    regenerateCSV(); // This will clear the file
    emit statusUpdate("❌ All selections cleared. CSV wiped.");
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

    // ✅ THE FIX: Clear previous red lines so ONLY the new selection is red!
    // (If you ever DO want to select multiple, just hold the SHIFT key on your keyboard)
    if (!(QApplication::keyboardModifiers() & Qt::ShiftModifier)) {
        clearSelections();
    }

    myContext->InitSelected();
    int addedCount = 0;

    QString xyzData;
    QTextStream stringOut(&xyzData);
    stringOut << "X,Y,Z,Rx,Ry,Rz\n";

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

        // Save the click to History
        myPathHistory.push_back({shape, plottedPath, resolution});
        addedCount++;

        // Run your math
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

    emit coordinatesExtracted(xyzData);
    regenerateCSV();
    emit statusUpdate(QString("✅ Extracted %1 new path(s). Total Paths in CSV: %2").arg(addedCount).arg(myPathHistory.size()));

    // ✅ ADD THIS LINE: Tells the UI to disable the "GET POINTS" button again
    emit selectionChanged(false);
}

void OcctWidget::regenerateCSV()
{
    QFile file(myCSVPath);
    // WriteOnly + Truncate means it completely overwrites the old file instantly
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qWarning() << "Could not open file for writing:" << myCSVPath;
        return;
    }

    QTextStream out(&file);
    out << "X,Y,Z,Rx,Ry,Rz\n";

    // =================================================================
    // ✅ THE FIX: QML SCALE FACTOR
    // 0.001 converts OCCT (mm) to QML (meters).
    // If QML still looks wrong, you can change this to 0.1, 10.0, etc.
    // =================================================================
    const double QML_SCALE_FACTOR = 0.001;

    // Create a 3D scaling transformation matrix originating from (0,0,0)
    gp_Trsf scaleTransform;
    scaleTransform.SetScale(gp_Pnt(0, 0, 0), QML_SCALE_FACTOR);

    // Play back the entire history stack to generate the perfect CSV state
    for (const auto& step : myPathHistory) {

        // 1. Scale the shape in memory specifically for the CSV output
        BRepBuilderAPI_Transform scaler(step.shape, scaleTransform);
        TopoDS_Shape scaledShape = scaler.Shape();

        // 2. Scale the resolution (distance) so the point density remains exactly the same
        double scaledResolution = step.resolution * QML_SCALE_FACTOR;

        // 3. Process the SCALED shape instead of the original one
        switch (scaledShape.ShapeType()) {
        case TopAbs_FACE:
            processFace(TopoDS::Face(scaledShape), out, scaledResolution);
            break;
        case TopAbs_WIRE:
            processWire(TopoDS::Wire(scaledShape), out, scaledResolution);
            break;
        case TopAbs_EDGE:
            processEdge(TopoDS::Edge(scaledShape), out, scaledResolution);
            break;
        default:
            break;
        }
    }

    file.close();
}

// ====================================================================
static gp_Dir g_faceNormal(0, 0, 1); // Default pointing UP
static bool g_hasFaceNormal = false;
void OcctWidget::processFace(const TopoDS_Face& face, QTextStream& out, double resolution)
{
    // 🚀 THE FIX: முகத்தின் செங்குத்து திசையை (Surface Normal) கண்டுபிடிக்கிறோம்
    Standard_Real umin, umax, vmin, vmax;
    BRepTools::UVBounds(face, umin, umax, vmin, vmax);
    Handle(Geom_Surface) surf = BRep_Tool::Surface(face);

    gp_Pnt p; gp_Vec d1u, d1v;
    surf->D1((umin + umax) / 2.0, (vmin + vmax) / 2.0, p, d1u, d1v);

    gp_Vec normVec = d1u.Crossed(d1v);
    if (face.Orientation() == TopAbs_REVERSED) normVec.Reverse();

    // நார்மல் திசையை சேவ் செய்கிறோம் (பக்கவாட்டுச் சுவரா, மேல்தளமா என்று அறிய)
    g_faceNormal = gp_Dir(normVec);
    g_hasFaceNormal = true;

    TopExp_Explorer wireExplorer(face, TopAbs_WIRE);
    int loopCount = 1;
    for (; wireExplorer.More(); wireExplorer.Next()) {
        TopoDS_Wire wire = TopoDS::Wire(wireExplorer.Current());
        QString boundaryMarker = QString("--- NEW BOUNDARY LOOP %1 ---").arg(loopCount);
        out << boundaryMarker << "\n";
        processWire(wire, out, resolution);
        loopCount++;
    }
    g_hasFaceNormal = false; // Reset after finishing the face
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
            // 🚀 THE FIX: Point மற்றும் Tangent-ஐ எடுக்கிறோம்
            compCurve.D1(param, pt, tangentVec);

            // 1. Tool Direction (செங்குத்து திசை)
            gp_Dir x_axis(tangentVec);
            gp_Dir normal = g_hasFaceNormal ? g_faceNormal : gp_Dir(0, 0, 1);
            gp_Dir z_axis(-normal.X(), -normal.Y(), -normal.Z());

            if (z_axis.IsParallel(x_axis, 0.01)) {
                x_axis = gp_Dir(1, 0, 0);
                if (z_axis.IsParallel(x_axis, 0.01)) x_axis = gp_Dir(0, 1, 0);
            }

            gp_Dir y_axis = z_axis.Crossed(x_axis);
            x_axis = y_axis.Crossed(z_axis);

            // 2. Calculate Euler Angles
            gp_Ax3 toolPos(pt, z_axis, x_axis);
            gp_Trsf trsf;
            trsf.SetTransformation(toolPos, gp_Ax3(gp_Pnt(0,0,0), gp_Dir(0,0,1), gp_Dir(1,0,0)));

            Standard_Real rx, ry, rz;
            trsf.GetRotation().GetEulerAngles(gp_YawPitchRoll, rz, ry, rx);

            // 🚀 THE FIX: X,Y,Z மற்றும் Rx,Ry,Rz ஆகிய 6 மதிப்புகளையும் பிரிண்ட் செய்கிறோம்!
            out << pt.X() << "," << pt.Y() << "," << pt.Z() << ","
                << rx * (180.0/M_PI) << "," << ry * (180.0/M_PI) << "," << rz * (180.0/M_PI) << "\n";
        }
    }
}

void OcctWidget::processEdge(const TopoDS_Edge& edge, QTextStream& out, double resolution)
{
    Standard_Real first, last;
    Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, first, last);
    if (curve.IsNull()) return;

    BRepAdaptor_Curve adaptor(edge);
    GCPnts_UniformAbscissa discretizer(adaptor, resolution, first, last);

    if (discretizer.IsDone()) {
        for (int i = 1; i <= discretizer.NbPoints(); ++i) {
            Standard_Real param = discretizer.Parameter(i);

            gp_Pnt pt;
            gp_Vec tangentVec;
            adaptor.D1(param, pt, tangentVec); // புள்ளி மற்றும் நகரும் திசை (Tangent)

            // 1. Tool-இன் X, Y, Z திசைகளைக் கணக்கிடுதல்
            gp_Dir x_axis(tangentVec); // டூல் நகரும் திசை (Direction of travel)

            // முகம் (Face) செலக்ட் ஆகவில்லை என்றால் நேராக கீழே பார்க்கும்படி வைக்கிறோம்
            gp_Dir normal = g_hasFaceNormal ? g_faceNormal : gp_Dir(0, 0, 1);

            // 🚀 THE FIX: டூல் பொருளை நோக்கிக் குத்தாகப் பார்க்க வேண்டும் (Z_axis = -Normal)
            gp_Dir z_axis(-normal.X(), -normal.Y(), -normal.Z());

            // பாதுகாப்பிற்காக (Tangent மற்றும் Normal இணையாக இருந்தால்)
            if (z_axis.IsParallel(x_axis, 0.01)) {
                x_axis = gp_Dir(1, 0, 0);
                if (z_axis.IsParallel(x_axis, 0.01)) x_axis = gp_Dir(0, 1, 0);
            }

            gp_Dir y_axis = z_axis.Crossed(x_axis); // Y திசை
            x_axis = y_axis.Crossed(z_axis); // Orthogonal சரிபார்ப்பு

            // 2. KDL Euler Angles (Rx, Ry, Rz) உருவாக்குதல்
            gp_Ax3 toolPos(pt, z_axis, x_axis);
            gp_Trsf trsf;
            trsf.SetTransformation(toolPos, gp_Ax3(gp_Pnt(0,0,0), gp_Dir(0,0,1), gp_Dir(1,0,0)));

            Standard_Real rx, ry, rz;
            trsf.GetRotation().GetEulerAngles(gp_YawPitchRoll, rz, ry, rx);

            double rx_deg = rx * (180.0 / M_PI);
            double ry_deg = ry * (180.0 / M_PI);
            double rz_deg = rz * (180.0 / M_PI);

            // 3. X,Y,Z மற்றும் Rx,Ry,Rz ஆகிய 6 மதிப்புகளையும் CSV-க்கு அனுப்புதல்
            out << pt.X() << "," << pt.Y() << "," << pt.Z() << ","
                << rx_deg << "," << ry_deg << "," << rz_deg << "\n";
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
    if (myView.IsNull()) {
        QTimer::singleShot(100, this, &OcctWidget::loadDefaultRobot);
        return;
    }

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
    // 🚀 THE FIX: Change > 5 to > 6 to load all 7 parts (link0 to link6)
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

    // 2. Setup the file path (Make sure it points to your 'step1' folder)
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
        Handle(AIS_Triangulation) aisShape = new AIS_Triangulation(mesh);

        gp_Trsf zeroTrsf;
        myContext->SetLocation(aisShape, TopLoc_Location(zeroTrsf));

        Quantity_Color partColor;
        // ==========================================================
        // 🎨 COLOR FIX: Ensure link6 (the flange) gets a distinct color
        // ==========================================================
        if (myCurrentLoadIndex == 0) partColor = Quantity_NOC_GRAY30;          // Base
        else if (myCurrentLoadIndex == 6) partColor = Quantity_NOC_GRAY75;     // Flange (link6)
        else partColor = Quantity_Color(1.0, 0.4, 0.0, Quantity_TOC_RGB);      // Orange for links 1-5

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


void OcctWidget::setUserFrameOrigin(double ui_x, double ui_y, double ui_z)
{
    if (myContext.IsNull()) return;

    // 🚀 NO SWAPPING.
    myCustomOrigin = gp_Pnt(ui_x, ui_y, ui_z);

    if (!myLoadedPart.IsNull()) {
        gp_Trsf transform;
        transform.SetTranslation(gp_Vec(ui_x, ui_y, ui_z));
        myContext->SetLocation(myLoadedPart, TopLoc_Location(transform));
    }

    if (myUserFrameMarker.IsNull()) {
        myUserFrameMarker = createThickTriad(1.3);
        myContext->SetDisplayMode(myUserFrameMarker, 1, Standard_False);
        myContext->Display(myUserFrameMarker, Standard_False);
    }

    gp_Trsf markerTrsf;
    markerTrsf.SetTranslation(gp_Vec(ui_x, ui_y, ui_z));
    myContext->SetLocation(myUserFrameMarker, TopLoc_Location(markerTrsf));

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
    myToolShape->SetColor(Quantity_NOC_BLUE1);
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