// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#pragma once

#include <QtGlobal>

// Tells touchpad scrolling from mouse wheel scrolling on Windows, where both
// arrive as ordinary wheel messages. Times are Windows message times in
// milliseconds, which wrap around.
class TouchpadScrollClassifier
{
public:
    enum class Reason
    {
        Mouse,    // a mouse wheel
        Source,   // Windows reported the touchpad as the source
        Contact,  // the touchpad reported fingers at the same time
        Sequence, // inertia that continues touchpad scrolling
        Delta,    // fine-grained deltas from a computer with a touchpad
    };

    void setTouchpadPresent(bool present) { m_touchpadPresent = present; }
    void setRawInputWorks(bool works) { m_rawInputWorks = works; }

    // A raw input report of the touchpad: fingers are on it at that time.
    void contactReport(quint32 time)
    {
        m_lastContact = time;
        m_contactSeen = true;
        m_rawInputWorks = true;
    }

    Reason classifyWheel(quint32 time, int delta, bool sourceIsTouchpad)
    {
        const bool sameSequence =
          m_wheelSeen && quint32(time - m_lastWheel) < SEQUENCE_GAP_MS;
        Reason reason = Reason::Mouse;
        if (sourceIsTouchpad) {
            reason = Reason::Source;
        } else if (m_contactSeen && near(time, m_lastContact)) {
            reason = Reason::Contact;
        } else if (sameSequence && m_sequenceFromTouchpad) {
            // Scrolling continues briefly after the fingers lift.
            reason = Reason::Sequence;
        } else if (!m_rawInputWorks && m_touchpadPresent &&
                   delta % WHEEL_NOTCH != 0) {
            // Without the touchpad's raw input, rely on its fine deltas.
            reason = Reason::Delta;
        }
        // Only a touchpad verdict carries over, so a misjudged first event
        // is corrected by the next one.
        m_sequenceFromTouchpad = reason != Reason::Mouse;
        m_lastWheel = time;
        m_wheelSeen = true;
        return reason;
    }

private:
    static constexpr int WHEEL_NOTCH = 120;
    static constexpr qint32 CONTACT_WINDOW_MS = 250;
    static constexpr quint32 SEQUENCE_GAP_MS = 600;

    static bool near(quint32 first, quint32 second)
    {
        const auto difference = static_cast<qint32>(first - second);
        return difference < CONTACT_WINDOW_MS &&
               difference > -CONTACT_WINDOW_MS;
    }

    bool m_touchpadPresent = false;
    bool m_rawInputWorks = false;
    bool m_contactSeen = false;
    bool m_wheelSeen = false;
    bool m_sequenceFromTouchpad = false;
    quint32 m_lastContact = 0;
    quint32 m_lastWheel = 0;
};
