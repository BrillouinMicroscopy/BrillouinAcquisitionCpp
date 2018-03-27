#include "stdafx.h"
#include "acquisition.h"
#include "simplemath.h"
#include "logger.h"


Acquisition::Acquisition(QObject *parent)
	: QObject(parent) {
}

Acquisition::~Acquisition() {
}

void Acquisition::startAcquisition(std::string filename) {

	m_fileHndl = new StorageWrapper(0, filename, H5F_ACC_RDWR);
	// move h5bm file to separate thread
	m_storageThread.startWorker(m_fileHndl);

	std::string now = QDateTime::currentDateTime().toOffsetFromUtc(QDateTime::currentDateTime().offsetFromUtc())
		.toString(Qt::ISODate).toStdString();

	m_fileHndl->setDate(now);

	std::string commentIn = "Brillouin data";
	m_fileHndl->setComment(commentIn);

	m_fileHndl->setResolution("x", m_acqSettings.xSteps);
	m_fileHndl->setResolution("y", m_acqSettings.ySteps);
	m_fileHndl->setResolution("z", m_acqSettings.zSteps);

	int resolutionXout = m_fileHndl->getResolution("x");

	// Create position vector
	int nrPositions = m_acqSettings.xSteps * m_acqSettings.ySteps * m_acqSettings.zSteps;
	std::vector<double> positionsX(nrPositions);
	std::vector<double> positionsY(nrPositions);
	std::vector<double> positionsZ(nrPositions);
	std::vector<double> posX = simplemath::linspace(m_acqSettings.xMin, m_acqSettings.xMax, m_acqSettings.xSteps);
	std::vector<double> posY = simplemath::linspace(m_acqSettings.yMin, m_acqSettings.yMax, m_acqSettings.ySteps);
	std::vector<double> posZ = simplemath::linspace(m_acqSettings.zMin, m_acqSettings.zMax, m_acqSettings.zSteps);
	int ll = 0;
	for (int ii = 0; ii < m_acqSettings.zSteps; ii++) {
		for (int jj = 0; jj < m_acqSettings.xSteps; jj++) {
			for (int kk = 0; kk < m_acqSettings.ySteps; kk++) {
				positionsX[ll] = posX[jj];
				positionsY[ll] = posY[kk];
				positionsZ[ll] = posZ[ii];
				ll++;
			}
		}
	}

	int rank = 3;
	// For compatibility with MATLAB respect Fortran-style ordering: z, x, y
	hsize_t *dims = new hsize_t[rank];
	dims[0] = m_acqSettings.zSteps;
	dims[1] = m_acqSettings.xSteps;
	dims[2] = m_acqSettings.ySteps;

	m_fileHndl->setPositions("x", positionsX, rank, dims);
	m_fileHndl->setPositions("y", positionsY, rank, dims);
	m_fileHndl->setPositions("z", positionsZ, rank, dims);
	delete[] dims;

	// do actual measurement 
	m_fileHndl->startWritingQueues();
	std::vector<double> data(m_acqSettings.camera.roi.height * m_acqSettings.camera.roi.width);
	int rank_data = 3;
	hsize_t dims_data[3] = { m_acqSettings.camera.frameCount , m_acqSettings.camera.roi.height, m_acqSettings.camera.roi.width };
	for (int ii = 0; ii < m_acqSettings.zSteps; ii++) {
		for (int jj = 0; jj < m_acqSettings.xSteps; jj++) {
			for (int kk = 0; kk < m_acqSettings.ySteps; kk++) {
				// move stage to correct position, wait for it to finish

				// acquire image

				// asynchronously write image to disk
				// the datetime has to be set here, otherwise it would be determined by the time the queue is processed
				std::string date = QDateTime::currentDateTime().toOffsetFromUtc(QDateTime::currentDateTime().offsetFromUtc())
					.toString(Qt::ISODate).toStdString();
				IMAGE *img = new IMAGE(jj, kk, ii, rank, dims, date, data);
				m_fileHndl->m_payloadQueue.enqueue(img);
				std::string info = "Image acquired " + std::to_string(ii*(m_acqSettings.xSteps*m_acqSettings.ySteps) + jj * m_acqSettings.ySteps + kk);
				qInfo(logInfo()) << info.c_str();
			}
		}
	}

	//m_acqSettings.fileHndl->setBackgroundData(data, rank, dims_data, "2016-05-06T11:11:00+02:00");

	//m_acqSettings.fileHndl->setCalibrationData(1, data, rank, dims_data, "methanol", 3.799);
	//m_acqSettings.fileHndl->setCalibrationData(2, data, rank, dims_data, "water", 5.088);

	m_storageThread.exit();
	m_storageThread.wait();
	delete m_fileHndl;
	m_fileHndl = NULL;

	std::string info = "Acquisition finished.";
	qInfo(logInfo()) << info.c_str();
	emit(s_acqCalibrationRunning(FALSE));
}