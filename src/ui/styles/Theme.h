#pragma once

#include <QColor>
#include <QFont>
#include <QString>

namespace Theme {

// Palette Colors - Luxury Light Minimal Futuristic
namespace Colors {
    inline const QString CanvasBg = "#F8FAFC";        // Slate-50: Main application backdrop
    inline const QString SubtleBg = "#F1F5F9";        // Slate-100: Soft container / sidebar backdrop
    inline const QString CardBg = "#FFFFFF";          // Pure white surface
    inline const QString CardHover = "#FDFEFE";       // Ultra subtle card hover
    
    inline const QString BorderColor = "#E2E8F0";     // Slate-200: Default hairline border
    inline const QString BorderLight = "#F1F5F9";     // Slate-100: Divider
    inline const QString BorderHover = "#CBD5E1";     // Slate-300: Active hover border
    inline const QString BorderFocus = "#6366F1";     // Indigo-500: Focus ring
    
    inline const QString Primary = "#4F46E5";         // Indigo-600: Refined primary brand
    inline const QString PrimaryHover = "#4338CA";    // Indigo-700
    inline const QString PrimaryPressed = "#3730A3";  // Indigo-800
    inline const QString PrimaryLight = "#EEF2FF";    // Indigo-50: Soft pill / selected item bg
    inline const QString PrimaryLightHover = "#E0E7FF"; // Indigo-100
    
    inline const QString TextPrimary = "#0F172A";     // Slate-900: High-contrast body & titles
    inline const QString TextSecondary = "#475569";   // Slate-600: Descriptive copy & labels
    inline const QString TextTertiary = "#94A3B8";    // Slate-400: Placeholders & metadata
    inline const QString TextMuted = "#CBD5E1";       // Slate-300: Subtle hints
    
    inline const QString Success = "#10B981";         // Emerald-500
    inline const QString SuccessBg = "#ECFDF5";       // Emerald-50
    inline const QString Warning = "#F59E0B";         // Amber-500
    inline const QString WarningBg = "#FFFBEB";       // Amber-50
    inline const QString Danger = "#EF4444";          // Rose-500
    inline const QString DangerBg = "#FEF2F2";        // Rose-50
    inline const QString Info = "#0EA5E9";            // Sky-500
    inline const QString InfoBg = "#F0F9FF";          // Sky-50
}

// Category semantic tokens
namespace Categories {
    inline const QString Work = "#3B82F6";            // Blue
    inline const QString WorkBg = "#EFF6FF";
    inline const QString Study = "#8B5CF6";           // Purple
    inline const QString StudyBg = "#F5F3FF";
    inline const QString Personal = "#EC4899";        // Pink
    inline const QString PersonalBg = "#FDF2F8";
    inline const QString Health = "#10B981";          // Green
    inline const QString HealthBg = "#ECFDF5";
    inline const QString Other = "#64748B";           // Slate
    inline const QString OtherBg = "#F8FAFC";

    inline QString colorFor(int categoryIndex) {
        switch (categoryIndex) {
            case 0: return Work;
            case 1: return Study;
            case 2: return Personal;
            case 3: return Health;
            default: return Other;
        }
    }

    inline QString bgFor(int categoryIndex) {
        switch (categoryIndex) {
            case 0: return WorkBg;
            case 1: return StudyBg;
            case 2: return PersonalBg;
            case 3: return HealthBg;
            default: return OtherBg;
        }
    }
}

// Priority semantic tokens
namespace Priorities {
    inline const QString High = "#EF4444";
    inline const QString HighBg = "#FEF2F2";
    inline const QString Medium = "#F59E0B";
    inline const QString MediumBg = "#FFFBEB";
    inline const QString Low = "#10B981";
    inline const QString LowBg = "#ECFDF5";

    inline QString colorFor(int priorityIndex) {
        switch (priorityIndex) {
            case 2: return High;
            case 1: return Medium;
            default: return Low;
        }
    }

    inline QString bgFor(int priorityIndex) {
        switch (priorityIndex) {
            case 2: return HighBg;
            case 1: return MediumBg;
            default: return LowBg;
        }
    }
}

// Typography System
namespace Typography {
    inline const QString FontFamily = "-apple-system, BlinkMacSystemFont, 'SF Pro Display', 'Inter', 'Segoe UI', Roboto, sans-serif";
    inline const int SizeDisplay = 24;
    inline const int SizeTitle = 18;
    inline const int SizeCardTitle = 15;
    inline const int SizeBody = 13;
    inline const int SizeCaption = 11;
    inline const int SizeBadge = 11;
}

// Spacing System (Strict 4/8px Grid)
namespace Spacing {
    inline const int XS = 4;
    inline const int S = 8;
    inline const int M = 12;
    inline const int L = 16;
    inline const int XL = 20;
    inline const int XXL = 24;
    inline const int XXXL = 32;
}

// Border Radius System
namespace Radius {
    inline const int Button = 8;
    inline const int Chip = 6;
    inline const int Input = 8;
    inline const int Card = 12;
    inline const int Dialog = 16;
}

} // namespace Theme
