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

#include <map>
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
    gp_Pnt activeOrigin;
    QString approach; // 🚀 ADDED: Remembers the approach angle!
};

class OcctWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OcctWidget(QWidget *parent = nullptr);
    ~OcctWidget() override;

    // =========================================================
    // 🚀 MULTI-TASK CORE METHODS
    // =========================================================
    void setUserFrameState(int ufIndex, bool isActive, double x, double y, double z);
    void loadStepFile(const std::string& filePath, int ufIndex);
    void transformLoadedPart(int ufIndex, double dx, double dy, double dz, double rx, double ry, double rz);
    void clearLoadedPart(int ufIndex);
    bool hasLoadedPart() const { return !myLoadedParts.empty(); }

    QString getOriginText(int ufIndex) const;

    void reloadRobot(const QString& folderPath, const QString& linkPrefix, double bx, double bz, double az, double ez, double fx, double wx, double fy, bool isCobot = false);
    double m_rob_fy = 109.0; // 🚀 ADD THIS TO PRIVATE VARS
    void reloadRobot(const QString& folderPath);

    void clearMarks();

    // Defines whether this widget acts as the Main Left screen or the Isolated Right screen
    enum ViewRole { MainRole, SideRole };
    void setViewRole(ViewRole role) { myRole = role; }

    void loadDefaultRobot();

    // Shifts the loaded workpiece for calibration
    void offsetWorkpiece(int ufIndex, double dx, double dy, double dz);

    void setSelectionMode(int mode);
    void enableOriginSelectionMode();
    void resetOrigin(int ufIndex);
    void setUserFrameOrigin(int ufIndex, double ui_x, double ui_y, double ui_z);
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

    // =========================================================
    // 🚀 DYNAMIC EXTRACTION METHODS (MOVED TO PUBLIC)
    // =========================================================
    void processCurrentSelection(double resolution, const QString& approach = "Top");
    void processAllEdges(double resolution, int ufIndex = -1, const QString& approach = "Top");

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
    Handle(AIS_ColoredShape) myOrientationMarker;
    int m_loadSessionId = 0;
    int m_activeSession = 0;
    bool m_isCobot = false;
    // =========================================================
    // 🚀 MULTI-TASK MAPS (Replaces single instances)
    // =========================================================
    std::map<int, Handle(AIS_Shape)> myLoadedParts;
    std::map<int, Handle(AIS_ColoredShape)> myUserFrameMarkers;
    std::map<int, gp_Pnt> myUFOrigins;

    QString m_robotFolderPath = "/home/texsonics/Documents/toolocct/step1/";
    QString m_robotLinkPrefix = "link";
    double m_rob_bx = 155.0, m_rob_bz = 470.0, m_rob_az = 604.0;
    double m_rob_ez = 200.0, m_rob_fx = 640.5, m_rob_wx = 100.0;

    TopoDS_Edge m_customStartEdge;
    TopoDS_Edge m_customEndEdge;

    Handle(AIS_Shape) m_customStartMarker;
    Handle(AIS_Shape) m_customEndMarker;

    // Custom Trim Tracking Variables (Labels & Percentages)
    Handle(AIS_TextLabel) m_customStartLabel;
    Handle(AIS_TextLabel) m_customEndLabel;
    double m_trimStartPct = 0.0;
    double m_trimEndPct = 100.0;

    // Start Marker tracking variables
    Handle(AIS_Shape) myStartPointMarker;
    Handle(AIS_TextLabel) myStartLabel;
    bool m_isFirstPointFound = false;

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

    int myCurrentLoadIndex = -1;
    void loadNextRobotLink();
    ViewRole myRole = MainRole;
    int myCurrentSelectionMode = 1; // Remembers the dropdown state

    Handle(AIS_Shape) myTargetMarker;
    Handle(V3d_Viewer) myViewer;
    Handle(V3d_View) myView;
    Handle(AIS_InteractiveContext) myContext;
    Handle(OpenGl_GraphicDriver) myGraphicDriver;

    std::vector<Handle(AIS_InteractiveObject)> myRobotLinks;
    QPoint myLastMousePos;

    bool myIsSettingOriginMode = false;
    gp_Pnt myDefaultOrigin{0.0, 0.0, 0.0};
    gp_Pnt myCustomOrigin{0.0, 0.0, 0.0};
    Handle(AIS_Trihedron) myOriginMarker;

    // Variables to manage History and the CSV
    std::vector<PathData> myPathHistory;
    std::vector<PathData> myRedoStack;
    QString myCSVPath;

    void initOCCT();
    void drawRoomGrid();

    // Centralized file writer
    void regenerateCSV();

    // =========================================================
    // 🚀 INTERNAL MATH HELPERS (Keep these Private)
    // =========================================================
    void processFace(const TopoDS_Face& face, QTextStream& out, double resolution, const gp_Pnt& activeOrigin, const QString& approach);
    void processWire(const TopoDS_Wire& wire, QTextStream& out, double resolution, const gp_Pnt& activeOrigin, const QString& approach);
    void processEdge(const TopoDS_Edge& edge, QTextStream& out, double resolution, const gp_Pnt& activeOrigin, const QString& approach);
    void drawOrientationMarker(const gp_Ax3& pos);
};

#endif // OCCTWIDGET_H