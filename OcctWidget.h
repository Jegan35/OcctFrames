#ifndef OCCTWIDGET_H
#define OCCTWIDGET_H

#include <QWidget>
#include <AIS_InteractiveContext.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <TopExp_Explorer.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_CompCurve.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <BRep_Tool.hxx>
#include <AIS_ColoredShape.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <Prs3d_Drawer.hxx>
#include <XCAFPrs_AISObject.hxx>
#include <AIS_Trihedron.hxx>
#include <Geom_Axis2Placement.hxx>
#include <AIS_Shape.hxx>
#include <AIS_TextLabel.hxx>

#include <vector>

// ADD THIS TO KILL THE X11 MACRO CLASH
#ifdef None
#undef None
#endif

class QTextStream;

// Data structure to remember everything about a click
struct PathData {
    TopoDS_Shape shape;
    Handle(AIS_Shape) visualRedPath;
    double resolution;
};

class OcctWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OcctWidget(QWidget *parent = nullptr);
    ~OcctWidget() override;
    bool hasLoadedPart() const { return !myLoadedPart.IsNull(); }
    void processAllEdges(double resolution);
    void transformLoadedPart(double dx, double dy, double dz, double rx, double ry, double rz);
    void clearLoadedPart();
    void processCurrentSelection(double resolution);
    void setUserFrameOrigin(double x, double y, double z);
    // Add this new line right below it to read the origin:
    QString getOriginText() const;

    // Defines whether this widget acts as the Main Left screen or the Isolated Right screen
    enum ViewRole { MainRole, SideRole };
    void clearMarks();
    void setViewRole(ViewRole role) { myRole = role; }

    void loadStepFile(const std::string& filePath);
    void loadDefaultRobot();
    // Shifts the loaded workpiece for calibration
    void offsetWorkpiece(double dx, double dy, double dz);

    void setSelectionMode(int mode);
    void enableOriginSelectionMode();
    void resetOrigin();

    // Side panel display
    void displayIsolatedPart(const TopoDS_Shape& shape);

    // Public slots to trigger from MainWindow buttons
    void undoSelection();
    void redoSelection();
    void clearSelections();
    void updateRobotPosture(double j1, double j2, double j3, double j4, double j5, double j6);
    void loadToolShapeOnTip(const QString& toolName, double x, double y, double z);
    void clearToolShape();
    void drawTargetMarker(double x, double y, double z);
    void clearTargetMarker();
    void calculateCustomStartPoint(double percentage);
    void calculateCustomEndPoint(double percentage);

signals:
    void statusUpdate(const QString& msg);
    void partSelectedForIsolation(const TopoDS_Shape& shape);
    void coordinatesExtracted(const QString& xyzData);
    void selectionChanged(bool isSelected);
    void robotLoadComplete();
    void customStartPointCalculated(const QString& xyz);
    void customEndPointCalculated(const QString& xyz);

protected:
    QPaintEngine* paintEngine() const override { return nullptr; }

    void showEvent(QShowEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:

    TopoDS_Edge m_customStartEdge;
    TopoDS_Edge m_customEndEdge;

    Handle(AIS_Shape) m_customStartMarker;
    Handle(AIS_Shape) m_customEndMarker;

    // =========================================================
    // 🚀 NEW: Custom Trim Tracking Variables (Labels & Percentages)
    // =========================================================
    Handle(AIS_TextLabel) m_customStartLabel;
    Handle(AIS_TextLabel) m_customEndLabel;
    double m_trimStartPct = 0.0;
    double m_trimEndPct = 100.0;

    // 🚀 NEW: Start Marker tracking variables
    Handle(AIS_Shape) myStartPointMarker;
    Handle(AIS_TextLabel) myStartLabel;
    bool m_isFirstPointFound = false;

    // 🚀 NEW: Function to draw the marker
    void drawStartMarker(const gp_Pnt& pt);

    double m_toolOffsetX = 0.0;
    double m_toolOffsetY = 0.0;
    double m_toolOffsetZ = 0.0;
    Handle(AIS_Shape) myToolShape;
    gp_Trsf myLastTipTrsf;
    std::vector<gp_Pnt> myTrajectoryPoints;
    Handle(AIS_Shape) myTrajectoryShape;
    Handle(AIS_ColoredShape) myBaseTriad;
    Handle(AIS_ColoredShape) myTipTriad;
    Handle(AIS_ColoredShape) createThickTriad(double scale);
    Handle(AIS_ColoredShape) myUserFrameMarker;
    int myCurrentLoadIndex = -1;
    void loadNextRobotLink();
    ViewRole myRole = MainRole;
    int myCurrentSelectionMode = 1; // Remembers the dropdown state
    Handle(AIS_Shape) myTargetMarker;
    Handle(V3d_Viewer) myViewer;
    Handle(V3d_View) myView;
    Handle(AIS_InteractiveContext) myContext;
    Handle(OpenGl_GraphicDriver) myGraphicDriver;

    // Remembers the loaded table/workpiece so we can offset it
    Handle(AIS_InteractiveObject) myLoadedPart;
    std::vector<Handle(AIS_InteractiveObject)> myRobotLinks;
    QPoint myLastMousePos;

    bool myIsSettingOriginMode = false;
    gp_Pnt myCustomOrigin{0.0, 0.0, 0.0};
    gp_Pnt myDefaultOrigin{0.0, 0.0, 0.0};
    Handle(AIS_Trihedron) myOriginMarker;

    // Variables to manage History and the CSV
    std::vector<PathData> myPathHistory;
    std::vector<PathData> myRedoStack;
    QString myCSVPath;

    void initOCCT();
    void drawRoomGrid();

    // Centralized file writer
    void regenerateCSV();

    void processEdge(const TopoDS_Edge& edge, QTextStream& out, double resolution);
    void processWire(const TopoDS_Wire& wire, QTextStream& out, double resolution);
    void processFace(const TopoDS_Face& face, QTextStream& out, double resolution);
};

#endif // OCCTWIDGET_H