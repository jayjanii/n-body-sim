#define _USE_MATH_DEFINES
#include "Scenarios.h"
#include "Sphere.h"
#include <cmath>
#include <glm/glm.hpp>

static const float G_SIM = 4.0f * static_cast<float>(M_PI * M_PI);

const char* scenarioName(ScenarioType type) {
	switch (type) {
	case ScenarioType::SolarSystem:    return "Solar System";
	case ScenarioType::BinaryStars:    return "Binary Stars";
	case ScenarioType::EarthMoonSun:   return "Earth + Moon + Sun";
	case ScenarioType::Symmetric3BodyV1: return "Symmetric 3-Body Version 1";
	case ScenarioType::Symmetric3BodyV2: return "Symmetric 3-Body Version 2";
	case ScenarioType::Symmetric4BodyV1: return "Symmetric 4-Body Version 1";
	case ScenarioType::Symmetric4BodyV2: return "Symmetric 4-Body Version 2";
	case ScenarioType::Symmetric5Body: return "Symmetric 5-Body";
	default: return "Unknown";
	}
}

// ---------- helpers ----------

// Simple circular orbit in the XZ plane — used by non-solar-system scenarios
static void addPlanet(std::vector<std::unique_ptr<Object>>& objects,
	float orbitalRadius, float mass, float visualRadius,
	glm::vec3 color, float centralMass = 1.0f) {
	float v = sqrtf(G_SIM * centralMass / orbitalRadius);
	objects.push_back(std::make_unique<Sphere>(
		glm::vec3(orbitalRadius, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, v),
		mass, visualRadius, color));
}

// ---------- Keplerian orbital mechanics ----------

// Solve Kepler's equation  M = E - e*sin(E)  via Newton iteration
static float solveKepler(float e, float M) {
	float E = M;
	for (int iter = 0; iter < 100; iter++) {
		float dE = (M - E + e * sinf(E)) / (1.0f - e * cosf(E));
		E += dE;
		if (fabsf(dE) < 1e-9f) break;
	}
	return E;
}

// Convert J2000 Keplerian elements to a (position, velocity) state vector.
// Angles in degrees.  Output position in AU, velocity in AU/yr.
// Coordinate frame: XZ = ecliptic plane, Y = north ecliptic pole (up).
static std::pair<glm::vec3, glm::vec3> keplerState(
	float a, float e,
	float i_deg, float Omega_deg, float omega_deg, float M0_deg,
	float centralMass = 1.0f) {

	const float DEG = static_cast<float>(M_PI) / 180.0f;
	float incl  = i_deg     * DEG;
	float Omega = Omega_deg * DEG;
	float omega = omega_deg * DEG;
	float M0    = M0_deg    * DEG;

	// Eccentric anomaly → true anomaly (atan2 form avoids quadrant ambiguity)
	float E  = solveKepler(e, M0);
	float nu = 2.0f * atan2f(
		sqrtf(1.0f + e) * sinf(0.5f * E),
		sqrtf(1.0f - e) * cosf(0.5f * E));

	float p = a * (1.0f - e * e);          // semi-latus rectum
	float r = p / (1.0f + e * cosf(nu));   // radial distance

	// Perifocal unit vectors in standard astronomical ecliptic frame
	// (x = vernal equinox, z = north ecliptic pole)
	float cO = cosf(Omega), sO = sinf(Omega);
	float co = cosf(omega), so = sinf(omega);
	float ci = cosf(incl),  si = sinf(incl);

	glm::vec3 Q_ast( cO*co - sO*so*ci,  sO*co + cO*so*ci,  so*si);
	glm::vec3 P_ast(-cO*so - sO*co*ci, -sO*so + cO*co*ci,  co*si);

	// Remap astronomical (x, y, z) → OpenGL (x, z, y)
	// so the ecliptic plane stays XZ and the north pole points up (+Y).
	auto toGL = [](glm::vec3 v) { return glm::vec3(v.x, v.z, v.y); };
	glm::vec3 Q = toGL(Q_ast);
	glm::vec3 P = toGL(P_ast);

	float sqrt_mu_p = sqrtf(G_SIM * centralMass / p);

	glm::vec3 pos = r * (cosf(nu) * Q + sinf(nu) * P);
	glm::vec3 vel = sqrt_mu_p * (-sinf(nu) * Q + (e + cosf(nu)) * P);

	return { pos, vel };
}

// ---------- Solar System (Sun + 8 planets, real J2000 orbital elements) ----------

static void loadSolarSystem(std::vector<std::unique_ptr<Object>>& objects) {
	// Sun
	objects.push_back(std::make_unique<Sphere>(
		glm::vec3(0.0f), glm::vec3(0.0f), 1.0f, 0.25f,
		glm::vec3(1.0f, 0.9f, 0.3f), true));

	// a(AU)  e       i(°)   Ω(°)    ω(°)    M₀(°)   mass(M☉)  visR   color
	struct PlanetData { float a, e, i, Omega, omega, M0, mass, vr; glm::vec3 color; };
	const PlanetData planets[] = {
		{ 0.387f,  0.2056f,  7.005f,  48.331f,  29.124f, 174.796f, 1.66e-7f,  0.030f, { 0.7f, 0.7f, 0.7f } }, // Mercury
		{ 0.723f,  0.0068f,  3.395f,  76.680f,  55.186f,  50.115f, 2.45e-6f,  0.050f, { 0.9f, 0.8f, 0.5f } }, // Venus
		{ 1.000f,  0.0167f,  0.001f, -11.260f, 114.208f, 358.617f, 3.003e-6f, 0.055f, { 0.2f, 0.5f, 1.0f } }, // Earth
		{ 1.524f,  0.0934f,  1.850f,  49.558f, 286.502f,  19.373f, 3.213e-7f, 0.040f, { 0.9f, 0.3f, 0.2f } }, // Mars
		{ 5.203f,  0.0489f,  1.303f, 100.464f, 273.867f,  20.020f, 9.545e-4f, 0.120f, { 0.8f, 0.6f, 0.4f } }, // Jupiter
		{ 9.537f,  0.0565f,  2.489f, 113.665f, 339.392f, 317.021f, 2.858e-4f, 0.100f, { 0.9f, 0.8f, 0.6f } }, // Saturn
		{ 19.191f, 0.0463f,  0.773f,  74.006f,  96.998f, 142.238f, 4.366e-5f, 0.080f, { 0.6f, 0.8f, 0.9f } }, // Uranus
		{ 30.069f, 0.0100f,  1.770f, 131.784f, 276.336f, 267.767f, 5.15e-5f,  0.078f, { 0.3f, 0.4f, 0.9f } }, // Neptune
	};

	for (const auto& p : planets) {
		auto [pos, vel] = keplerState(p.a, p.e, p.i, p.Omega, p.omega, p.M0);
		objects.push_back(std::make_unique<Sphere>(pos, vel, p.mass, p.vr, p.color));
	}
}

// ---------- Binary Star System ----------

static void loadBinaryStars(std::vector<std::unique_ptr<Object>>& objects) {
	float sep = 1.0f;
	float m1 = 1.0f, m2 = 0.8f;
	float totalMass = m1 + m2;
	// Each star orbits the common center of mass
	float r1 = sep * m2 / totalMass;
	float r2 = sep * m1 / totalMass;
	float v1 = sqrtf(G_SIM * m2 * m2 / (totalMass * sep));
	float v2 = sqrtf(G_SIM * m1 * m1 / (totalMass * sep));

	objects.push_back(std::make_unique<Sphere>(
		glm::vec3(r1, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, v1),
		m1, 0.20f, glm::vec3(1.0f, 0.9f, 0.3f), true));

	objects.push_back(std::make_unique<Sphere>(
		glm::vec3(-r2, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -v2),
		m2, 0.18f, glm::vec3(1.0f, 0.5f, 0.2f), true));

	// A circumbinary planet
	float rPlanet = 3.0f;
	float vPlanet = sqrtf(G_SIM * totalMass / rPlanet);
	objects.push_back(std::make_unique<Sphere>(
		glm::vec3(rPlanet, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, vPlanet),
		3.0e-6f, 0.06f, glm::vec3(0.2f, 0.5f, 1.0f)));
}

// ---------- Earth + Moon + Sun ----------

static void loadEarthMoonSun(std::vector<std::unique_ptr<Object>>& objects) {
	// Sun
	objects.push_back(std::make_unique<Sphere>(
		glm::vec3(0.0f), glm::vec3(0.0f), 1.0f, 0.25f,
		glm::vec3(1.0f, 0.9f, 0.3f), true));

	// Earth at 1 AU
	float rE = 1.0f;
	float mE = 3.003e-6f;
	float vE = sqrtf(G_SIM / rE);
	objects.push_back(std::make_unique<Sphere>(
		glm::vec3(rE, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, vE),
		mE, 0.055f, glm::vec3(0.2f, 0.5f, 1.0f)));

	// Moon: ~0.00257 AU from Earth, mass ~3.694e-8 solar masses
	// Orbital velocity around Earth: v_moon = sqrt(G * mE / rMoon)
	// Then add Earth's velocity for the inertial frame
	float rMoon = 0.00257f;
	float mMoon = 3.694e-8f;
	float vMoonRel = sqrtf(G_SIM * mE / rMoon);
	objects.push_back(std::make_unique<Sphere>(
		glm::vec3(rE + rMoon, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, vE + vMoonRel),
		mMoon, 0.02f, glm::vec3(0.8f, 0.8f, 0.8f)));
}

// ---------- Symmetric N-body (regular polygon) ----------

static void loadSymmetric(std::vector<std::unique_ptr<Object>>& objects, int n) {
	// Equal masses on a regular polygon, all orbiting the center
	float mass = 1.0f;
	float radius = 1.5f;
	float visualR = 0.10f;

	// For a symmetric n-body on a circle:
	// The gravitational pull toward the center from (n-1) equal masses determines the orbital speed.
	// F_center = sum of forces projected toward center
	// v = sqrt(G * mass * S / radius) where S = sum_{k=1}^{n-1} 1/(2*sin(pi*k/n)) * cos component
	// Simplified: compute the net inward acceleration and set v = sqrt(a_net * radius)
	float accelInward = 0.0f;
	for (int k = 1; k < n; k++) {
		float angle = 2.0f * static_cast<float>(M_PI) * k / n;
		// Distance between body 0 and body k on a unit circle of radius R
		float dist = 2.0f * radius * sinf(static_cast<float>(M_PI) * k / n);
		// Force magnitude (per unit mass, from body k): G * mass / dist^2
		// Component toward center:
		float fx = G_SIM * mass * (cosf(angle) - 1.0f) / (dist * dist * dist) * 2.0f * radius * (cosf(angle) - 1.0f);
		float fy = G_SIM * mass * sinf(angle) / (dist * dist * dist) * 2.0f * radius * sinf(angle);
		// Actually let me compute this more carefully
		// Position of body 0: (R, 0)
		// Position of body k: (R*cos(theta_k), R*sin(theta_k))
		// Direction from 0 to k: (R*cos(theta_k) - R, R*sin(theta_k))
		// |dir| = dist (computed above)
		// Force on 0 from k: G*m/dist^2 * dir_normalized
		// Radial component toward center = -dot(force, r_hat) where r_hat = (1,0) for body 0
		float dx = radius * cosf(angle) - radius;
		float dz = radius * sinf(angle);
		float d = sqrtf(dx * dx + dz * dz);
		// Force toward center component (negative x for body at +x)
		accelInward += G_SIM * mass * (-dx) / (d * d * d) * (-1.0f); // wait, let me be more careful
	}

	// Let me just compute it properly: net acceleration on body 0 at (R, 0, 0)
	// toward center is in -x direction
	float ax_total = 0.0f;
	for (int k = 1; k < n; k++) {
		float angle = 2.0f * static_cast<float>(M_PI) * k / n;
		float dx = radius * (cosf(angle) - 1.0f);
		float dz = radius * sinf(angle);
		float d2 = dx * dx + dz * dz;
		float d = sqrtf(d2);
		// acceleration on body 0 from body k, x component
		ax_total += G_SIM * mass * dx / (d * d2);
	}
	// ax_total should be negative (pointing toward center since body is at +x)
	// Centripetal: v^2/R = |ax_total|
	float speed = sqrtf(fabsf(ax_total) * radius);

	// Color palette
	glm::vec3 colors[] = {
		glm::vec3(1.0f, 0.3f, 0.3f),
		glm::vec3(0.3f, 1.0f, 0.3f),
		glm::vec3(0.3f, 0.3f, 1.0f),
		glm::vec3(1.0f, 1.0f, 0.3f),
		glm::vec3(1.0f, 0.3f, 1.0f),
	};

	for (int i = 0; i < n; i++) {
		float angle = 2.0f * static_cast<float>(M_PI) * i / n;
		float px = radius * cosf(angle);
		float pz = radius * sinf(angle);
		// Velocity perpendicular to radius (tangential, counter-clockwise in XZ)
		float vx = -speed * sinf(angle);
		float vz =  speed * cosf(angle);

		objects.push_back(std::make_unique<Sphere>(
			glm::vec3(px, 0.0f, pz),
			glm::vec3(vx, 0.0f, vz),
			mass, visualR,
			colors[i % 5]));
	}
}

static void loadSitnikov(std::vector<std::unique_ptr<Object>>& objects) {
	float massBinaries = 1.0f;
	float massOscillator = 0.0001f; // Negligible mass to keep the XZ plane perfectly stationary
	float radius = 1.5f;
	float visualR = 0.10f;
	float dropHeight = 3.0f; // Starting height of the oscillator on the Y-axis

	// Calculate the orbital speed for the two massive bodies.
	// Force between them: F = G * m1 * m2 / (2R)^2
	// Centripetal acceleration: v^2 / R = F / m -> v = sqrt(G * mass / (4R))
	float speed = sqrtf(G_SIM * massBinaries / (4.0f * radius));

	glm::vec3 colors[] = {
		glm::vec3(1.0f, 0.3f, 0.3f), // Red
		glm::vec3(0.3f, 0.3f, 1.0f), // Blue
		glm::vec3(1.0f, 1.0f, 0.3f)  // Yellow (Oscillator)
	};

	// Body 0: Massive body at +X, moving in +Z
	objects.push_back(std::make_unique<Sphere>(
		glm::vec3(radius, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, speed),
		massBinaries, visualR, colors[0], true));

	// Body 1: Massive body at -X, moving in -Z
	objects.push_back(std::make_unique<Sphere>(
		glm::vec3(-radius, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, -speed),
		massBinaries, visualR, colors[1], true));

	// Body 2: Oscillator on the +Y axis, dropped from rest
	objects.push_back(std::make_unique<Sphere>(
		glm::vec3(0.0f, dropHeight, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		massOscillator, visualR * 0.5f, colors[2])); // Slightly smaller visually
}

static void loadOrthogonalBinaries(std::vector<std::unique_ptr<Object>>& objects) {
	float mass = 1.0f;
	float radius = 1.5f;
	float visualR = 0.10f;

	// Calculate net inward acceleration on a body at (R, 0, 0)
	// 1. Pull from its direct partner at (-R, 0, 0):
	float a_partner = (G_SIM * mass) / (4.0f * radius * radius);

	// 2. Pull from the two orthogonal bodies at (0, R, 0) and (0, -R, 0):
	// Distance to orthogonal bodies is R*sqrt(2).
	// The force magnitude is G*M / (2R^2). 
	// The inward radial component is projected by multiplying by cos(45 deg) = 1/sqrt(2).
	// Since there are two orthogonal bodies, we multiply by 2.
	float a_orthogonal = 2.0f * ((G_SIM * mass) / (2.0f * radius * radius)) * (1.0f / sqrtf(2.0f));

	// Total inward acceleration dictates the necessary orbital speed
	float a_total = a_partner + a_orthogonal;
	float speed = sqrtf(a_total * radius);

	glm::vec3 colors[] = {
		glm::vec3(1.0f, 0.3f, 0.3f), // Red (Pair 1)
		glm::vec3(1.0f, 0.3f, 0.3f), // Red (Pair 1)
		glm::vec3(0.3f, 1.0f, 0.3f), // Green (Pair 2)
		glm::vec3(0.3f, 1.0f, 0.3f)  // Green (Pair 2)
	};

	// --- PAIR 1: Orbiting in the XZ plane ---
	// Body 0: at +X, moving in +Z
	objects.push_back(std::make_unique<Sphere>(
		glm::vec3(radius, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, speed),
		mass, visualR, colors[0], true));

	// Body 1: at -X, moving in -Z
	objects.push_back(std::make_unique<Sphere>(
		glm::vec3(-radius, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, -speed),
		mass, visualR, colors[1], true));

	// --- PAIR 2: Orbiting in the XY plane ---
	// Body 2: at +Y, moving in +X
	objects.push_back(std::make_unique<Sphere>(
		glm::vec3(0.0f, radius, 0.0f),
		glm::vec3(speed, 0.0f, 0.0f),
		mass, visualR, colors[2], true));

	// Body 3: at -Y, moving in -X
	objects.push_back(std::make_unique<Sphere>(
		glm::vec3(0.0f, -radius, 0.0f),
		glm::vec3(-speed, 0.0f, 0.0f),
		mass, visualR, colors[3], true));
}

// ---------- public loader ----------

void loadScenario(ScenarioType type,
	std::vector<std::unique_ptr<Object>>& objects) {
	objects.clear();

	switch (type) {
	case ScenarioType::SolarSystem:    loadSolarSystem(objects);  break;
	case ScenarioType::BinaryStars:    loadBinaryStars(objects);  break;
	case ScenarioType::EarthMoonSun:   loadEarthMoonSun(objects); break;
	case ScenarioType::Symmetric3BodyV1: loadSymmetric(objects, 3); break;
	case ScenarioType::Symmetric3BodyV2: loadSitnikov(objects); break;
	case ScenarioType::Symmetric4BodyV1: loadSymmetric(objects, 4); break;
	case ScenarioType::Symmetric4BodyV2: loadOrthogonalBinaries(objects); break;
	case ScenarioType::Symmetric5Body: loadSymmetric(objects, 5); break;
	default: loadSolarSystem(objects); break;
	}
}
